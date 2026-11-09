#include <cassert>
#include <iostream>
#include "Profiler.h"
#include "StreamManager.h"

//#define HOSTADDR "192.168.1.2"  // MacBook on Wi-Fi LAN...
#define TCPPORT 50000
#define UDPPORT 50001

StreamManager::StreamManager():
  connected(false),
  myIP(),
  myToken(0),
  myReqstTID(0), myRespTID(0),
  myTCPSendQueue(), myUDPSendQueue(),
  myTCPReceiveQueue(), myUDPReceiveQueue(),
  myTCPDefaultServ(myTCPSendQueue, myTCPReceiveQueue),  // The queues won't be used in this case...
  myTCPServ(myTCPSendQueue, myTCPReceiveQueue),
  myUDPServ(myUDPSendQueue, myUDPReceiveQueue),
  _stream_protocol(UDP){
  std::cout << "Constructing StreamManager class..." << '\n';
}

StreamManager::~StreamManager() {
  std::cout << "Destructing StreamManager class..." << '\n';
}

int StreamManager::setIPAddress(const std::string& ip) {
  if (!(connected)) {
    myIP = ip;
    return 0;
  }
  return -1;
}

int StreamManager::connect() {
  if (!(connected)) {
    try {
        myTCPDefaultServ.configure(myIP.c_str(), TCPPORT);
        myTCPDefaultServ.connect();
        myToken = myTCPDefaultServ.requestEntry();
        myTCPDefaultServ.disconnect();
      if (_stream_protocol == TCP) {
        myTCPServ.configure(myIP.c_str(), TCPPORT); // TODO Improve exception management...
        myTCPServ.connect();
        myTCPServ.startListen();
      } else {
        myUDPServ.configure(myIP.c_str(), UDPPORT);
        myUDPServ.startListen();
      }
    } catch (TCPConnectionException& e) {
      std::cerr << "Unable to finalize connection: " << e.what() << std::endl;
      myTCPDefaultServ.disconnect();
      if (_stream_protocol == TCP) {
        myTCPServ.disconnect();
        myTCPServ.stopListen();
      } else {
        myUDPServ.stopListen();
      }
      return -1;
    }
    connected = true;
  }
  return 0;
}

int StreamManager::disconnect() {
  if (connected) {
    try {
        myTCPDefaultServ.configure(myIP.c_str(), TCPPORT);
        myTCPDefaultServ.connect();
        myTCPDefaultServ.requestExit(myToken);
        myTCPDefaultServ.disconnect();
      if (_stream_protocol == TCP) {
        myTCPServ.disconnect();
        myTCPServ.stopListen();
      } else {
        myUDPServ.stopListen();
      }
    } catch (TCPConnectionException& e) {
      std::cerr << "Unable to finalize disconnection: " << e.what() << std::endl;
      myTCPDefaultServ.disconnect();
      if (_stream_protocol == TCP) {
        myTCPServ.disconnect();
        myTCPServ.stopListen();
      } else {
        myUDPServ.stopListen();
      }
      return -1;
    }
  }
  connected = false;
  myTCPSendQueue.clear();
  myTCPReceiveQueue.clear();
  myUDPSendQueue.clear();
  myUDPReceiveQueue.clear();
  return 0;
}

void StreamManager::sendStream(AudioVector& v) {
    if (connected) {
        // Profiler::add_record(myToken, ++myReqstTID, Profiler::OperationID::REQUEST_DATA_GATHERING); TODO
        StreamVector sv;
        // DefaultTimePoint call_time = SystemClock::now();
        ClientTID reqstTID = ++myReqstTID;
        sv.setTID(reqstTID);
        sv.setAudioVector(v);
        sv.addReadPermission(myToken);
        sv.setOwner(myToken);
        sv.set_flag(_flag);
        for (int i = 0; i < redundancy; i++) {
            // myUDPServ.sendStream(myToken, reqstTID, v);
            // sv.setTID(reqstTID);
            // sv.setAudioVector(v);
            // sv.addReadPermission(myToken);
            // sv.setOwner(myToken);
            if (get_audio_stream_protocol() == TCP) {
                myTCPSendQueue.insertVector(myToken, sv);
            } else {
                myUDPSendQueue.insertVector(myToken, sv);
            }
        }
        Profiler::add_record(myToken, myReqstTID, Profiler::OperationID::REQUEST_PENDING);
        // assert(rtt_map.count(reqstTID) == 0);
        // rtt_map[reqstTID].send_time = call_time;
    }
}

AudioVector StreamManager::retrieveStream() {
  if (connected) {
      // ClientTID respTID = ++myRespTID;  // TODO
      //return myOutQueue.retrieveVector(myToken);
      StreamVector sv;
      // SystemTime t0 = SystemClock::now();
      int res = 0;
      if (get_audio_stream_protocol() == TCP) {
          res = myTCPReceiveQueue.retrieveVector(sv, false);
      } else {
          res = myUDPReceiveQueue.retrieveVector(sv, false);
      }
      /*
      SystemTime t1 = SystemClock::now();
      std::cout << std::fixed;
      std::cout << "retrieveVector() execution (from outside): " << std::chrono::duration_cast<Nanoseconds>(t1.time_since_epoch() - t0.time_since_epoch()).count() * 1e-6 << std::endl;
      */
      if (res) {
          return AudioVector(AUDIO_VECTOR_SIZE * NUM_CHANNELS, 0.0);
      }
      Profiler::add_record(myToken, sv.tid(), Profiler::OperationID::RESPONSE_COMPUTING);
      AudioVector result = sv.readVector(myToken);
      // DefaultTimePoint call_time = SystemClock::now();
      // assert(rtt_map.count(respTID) == 1);
      // rtt_map[respTID].receive_time = call_time;
      Profiler::add_record(myToken, sv.tid(), Profiler::OperationID::RESPONSE_DONE);
      return result;
  } else {
      return AudioVector(AUDIO_VECTOR_SIZE * NUM_CHANNELS, 0.0);
  }
}

AudioVector StreamManager::retrieveStreamWithLatency() {
    if (connected) {
        if (get_audio_stream_protocol() == TCP) {
            return myTCPReceiveQueue.retrieveVector(myToken);
        } else {
            return myUDPReceiveQueue.retrieveVector(myToken);
        }
    }
    return AudioVector(AUDIO_VECTOR_SIZE * NUM_CHANNELS, 0.0);
}

void StreamManager::setMaxLatency(int blocks) {
    myTCPSendQueue.setMaxLength(blocks);
    myUDPSendQueue.setMaxLength(blocks);
    myTCPReceiveQueue.setMaxLength(blocks);
    myUDPReceiveQueue.setMaxLength(blocks);
}

void StreamManager::set_audio_stream_protocol(AudioStreamProtocol proto) {
    _stream_protocol = proto;
}

AudioStreamProtocol StreamManager::get_audio_stream_protocol() const {
    return _stream_protocol;
}

void StreamManager::set_flag(Flag f) {
    _flag = f;
    myTCPServ.set_mode(f);
}

Flag StreamManager::get_flag() const {
    return _flag;
}

RTTStatistics StreamManager::getRTTStatistics() {
    uint count = 0;
    float total_duration = 0.0f;
    float minimum = 1000.0f;
    float maximum = 0.0f;
    for (std::map<uint64_t, RTTData>::iterator it = rtt_map.begin(); it != rtt_map.end(); it++) {
        float duration = std::chrono::duration_cast<std::chrono::seconds>(
            it->second.receive_time - it->second.send_time
        ).count();
        count++;
        total_duration += duration;
        minimum = duration < minimum ? duration : minimum;
        maximum = duration > maximum ? duration : maximum;
    }
    return {
        total_duration / (float) count,
        minimum,
        maximum
    };
}
