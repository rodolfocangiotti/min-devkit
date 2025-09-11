#ifndef STREAM_MANAGER_H
#define STREAM_MANAGER_H

#include <string>
#include "StreamQueue.h"
#include "UDPConnection.h"
#include "TCPConnection.h"
#include "types.h"

class StreamManager {
public:
  StreamManager();
  ~StreamManager();
  int setIPAddress(const std::string& ip);
  int connect();
  int disconnect();
  void sendStream(AudioVector& v);
  AudioVector retrieveStream();
private:
  bool connected;
  std::string myIP;
  ClientTID myReqstTID;
  ClientTID myRespTID;
  ClientToken myToken;
  StreamQueue myQueue;
  TCPConnection myTCPServ;
  UDPConnection myUDPServ;
};

#endif
