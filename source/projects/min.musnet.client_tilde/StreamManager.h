#ifndef STREAM_MANAGER_H
#define STREAM_MANAGER_H

#include <string>
#include "StreamQueue.h"
#include "UDPConnection.h"
#include "TCPConnection.h"
#include "types.h"

struct RTTData {
    SystemTime send_time;
    SystemTime receive_time;
};

struct RTTStatistics {
    float average;
    float minimum;
    float maximum;
};

class StreamManager {
public:
    StreamManager();
    ~StreamManager();
    int setIPAddress(const std::string& ip);
    int connect();
    int disconnect();
    void setMaxLatency(int blocks);
    void sendStream(AudioVector& v);
    void set_audio_stream_protocol(AudioStreamProtocol proto);
    AudioStreamProtocol get_audio_stream_protocol() const;
    void set_flag(Flag f);
    Flag get_flag() const;
    AudioVector retrieveStream();
    AudioVector retrieveStreamWithLatency();
    RTTStatistics getRTTStatistics();
private:
    bool connected;
    std::string myIP;
    ClientTID myReqstTID;
    ClientTID myRespTID;
    ClientToken myToken;
    StreamQueue myTCPSendQueue, myUDPSendQueue;
    StreamQueue myTCPReceiveQueue, myUDPReceiveQueue;
    TCPConnection myTCPDefaultServ;
    TCPConnection myTCPServ;
    UDPConnection myUDPServ;
    const int redundancy = REDUNDANCY;
    AudioStreamProtocol _stream_protocol;
    Flag _flag;
    std::map<uint64_t, RTTData> rtt_map;
};

#endif
