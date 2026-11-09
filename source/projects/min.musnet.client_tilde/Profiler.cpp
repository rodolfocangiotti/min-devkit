#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <unordered_map>
#include "Profiler.h"

bool is_increasing_transaction_id(Profiler::Record& first, Profiler::Record& second) {
    return first.transaction < second.transaction;
}

bool is_increasing_operation_id(Profiler::Record& first, Profiler::Record& second) {
    return first.operation < second.operation;
}

Profiler::Record::Record():
    token(),
    transaction(),
    operation(),
    timestamp() {

}

Profiler::Record::Record(ClientToken token, ClientTID transaction, OperationID operation, Timestamp timestamp):
    token(token),
    transaction(transaction),
    operation(operation),
    timestamp(timestamp) {

}

int Profiler::add_record(ClientToken token, ClientTID transaction, OperationID operation) {
    HighResolutionTime t = HighResolutionClock::now();
    Timestamp timestamp = std::chrono::duration_cast<Nanoseconds>(t.time_since_epoch()).count();
    {
        Locker lock_guard(_mutex);
        _records.emplace_back(token, transaction, operation, timestamp);
    }
}

Profiler::Timestamp Profiler::get_timestamp(ClientToken token, ClientTID transaction, OperationID operation) {
    Locker lock_guard(_mutex);
    for (RecordLog::iterator it = _records.begin(); it != _records.end(); it++) {
        if (
            (it->token == token) &&
            (it->transaction == transaction) &&
            (it->operation == operation)
        ) {
            return it->timestamp;
        }
    }
    return 0;
}

std::string Profiler::get_statistics() {
    // This function doesn't need to be optimized in terms of performance.
    typedef std::list<Record> RecordList;
    typedef std::unordered_map<ClientToken, RecordList> RecordMap;
    RecordMap record_map;
    {
        Locker lock_guard(_mutex);
        for (RecordLog::iterator it = _records.begin(); it != _records.end(); it++) {
            Record record = *it;
            record_map[record.token].emplace_back(record);
        }
    }
    uint64_t to_request_pending_count = 0;
    double total_to_request_pending = 0.0;
    uint64_t to_request_start_count = 0;
    double total_to_request_start = 0.0;
    uint64_t to_udp_dispatch_count = 0;
    double total_to_udp_dispatch = 0.0;
    uint64_t to_udp_receipt_count = 0;
    double total_to_udp_receipt = 0.0;
    uint64_t to_response_pending_count = 0;
    double total_to_response_pending = 0.0;
    uint64_t to_response_computing_count = 0;
    double total_to_response_computing = 0.0;
    uint64_t to_response_done_count = 0;
    double total_to_response_done = 0.0;
    
    uint64_t rtt_count = 0;
    double total_rtt = 0.0;
    
    std::stringstream result;
    result << std::fixed;

    for (RecordMap::iterator map_it = record_map.begin(); map_it != record_map.end(); map_it++) {
        RecordList& client_records = map_it->second;
        client_records.sort(is_increasing_operation_id);
        client_records.sort(is_increasing_transaction_id);
        Record previous_entry;
        Record data_gathering_mem;
        for (RecordList::iterator list_it = client_records.begin(); list_it != client_records.end(); list_it++) {
            assert(map_it->first == list_it->token);
            double t_delta = static_cast<double>(list_it->timestamp) -
                             static_cast<double>(previous_entry.timestamp);

            if (t_delta < 0) {
                // if (list_it->operation != UDP_RECEIPT) {
                //     result << "**************" << std::endl;
                //     result << previous_entry.token << ',' << previous_entry.transaction << ',' << previous_entry.operation << ',' << previous_entry.timestamp << std::endl;
                //     result << list_it->token << ',' << list_it->transaction << ',' << list_it->operation << ',' << list_it->timestamp << std::endl;
                //     result << t_delta * 1e-9 << " - " << list_it->operation << std::endl;
                    std::cout << t_delta * 1e-6 << std::endl;
                // }
            }

            if (list_it->operation == REQUEST_DATA_GATHERING) {
                data_gathering_mem = *list_it;
            } else if (list_it->operation == REQUEST_PENDING) {
                if (previous_entry.operation == REQUEST_DATA_GATHERING) {
                    assert(list_it->transaction == previous_entry.transaction);
                    to_request_pending_count++;
                    total_to_request_pending += t_delta;
                }
            } else if (list_it->operation == REQUEST_START) {
                if (previous_entry.operation == REQUEST_PENDING) {
                    assert(list_it->transaction == previous_entry.transaction);
                    to_request_start_count++;
                    total_to_request_start += t_delta;
                }
            } else if (list_it->operation == UDP_DISPATCH) {
                if (previous_entry.operation == REQUEST_START) {
                    assert(list_it->transaction == previous_entry.transaction);
                    to_udp_dispatch_count++;
                    total_to_udp_dispatch += t_delta;
                }
            } else if (list_it->operation == UDP_RECEIPT) {
                if (previous_entry.operation == UDP_DISPATCH) {
                    assert(list_it->transaction == previous_entry.transaction);
                    to_udp_receipt_count++;
                    total_to_udp_receipt += t_delta;
                }
            } else if (list_it->operation == RESPONSE_PENDING) {
                if (previous_entry.operation == UDP_RECEIPT) {
                    assert(list_it->transaction == previous_entry.transaction);
                    to_response_pending_count++;
                    total_to_response_pending += t_delta;
                }
            } else if (list_it->operation == RESPONSE_COMPUTING) {
                if (previous_entry.operation == RESPONSE_PENDING) {
                    assert(list_it->transaction == previous_entry.transaction);
                    to_response_computing_count++;
                    total_to_response_computing += t_delta;
                }
            } else if (list_it->operation == RESPONSE_DONE) {
                if (previous_entry.operation == RESPONSE_COMPUTING) {
                    assert(list_it->transaction == previous_entry.transaction);
                    to_response_done_count++;
                    total_to_response_done += t_delta;
                }
                rtt_count++;
                total_rtt += static_cast<double>(list_it->timestamp) - static_cast<double>(data_gathering_mem.timestamp);
            }
            previous_entry = *list_it;
        }
    }
    result << "Request Data Gathering - Request Pending -- average: " << (total_to_request_pending / to_request_pending_count) * 1e-6 << std::endl;
    result << "Request Pending - Request Start -- average: " << (total_to_request_start / to_request_start_count) * 1e-6 << std::endl;
    result << "Request Start - UDP Dispatch -- average: " << (total_to_udp_dispatch / to_udp_dispatch_count) * 1e-6 << std::endl;
    result << "UDP Dispatch - UDP Receipt -- average: " << (total_to_udp_receipt / to_udp_receipt_count) * 1e-6 << std::endl;
    result << "UDP Receipt - Response Pending -- average: " << (total_to_response_pending / to_response_pending_count) * 1e-6 << std::endl;
    result << "Response Pending - Response Computing -- average: " << (total_to_response_computing / to_response_computing_count) * 1e-6 << std::endl;
    result << "Response Computing - Response Done -- average: " << (total_to_response_done / to_response_done_count) * 1e-6 << std::endl;
    result << std::endl;
    result << "RTT -- average: " << (total_rtt / rtt_count) * 1e-6 << std::endl;
    return result.str();
}

void Profiler::clear_records() {
    _records.clear();
}

void Profiler::export_records(std::string file_path) {
    // TODO
}

std::mutex Profiler::_mutex;
Profiler::RecordLog Profiler::_records;

/*
UDP Receipt - Response Pending -- average: 0.008756
Response Pending - Response Start -- average: 3.538991
Response Start - Response End -- average: 0.041701
Response End - Dispatch Pending -- average: 0.008422
Dispatch Pending - Dispatch Start -- average: 0.073333
Dispatch Start - UDP Dispatch -- average: 0.037418
*/
