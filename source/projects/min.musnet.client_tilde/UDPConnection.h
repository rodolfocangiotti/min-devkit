#ifndef UDP_CONNECTION_H
#define UDP_CONNECTION_H

#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "StreamQueue.h"
#include "UDPDatagram.h"
#include "types.h"

#if defined(__APPLE__) || defined(__linux__)
#include <arpa/inet.h>
#elif defined(_WIN32)
#include <winsock2.h>
#include <WS2tcpip.h>
#endif

class UDPConnection {
public:
  UDPConnection(StreamQueue& sq);
  ~UDPConnection();
  // ********************
  void configure(Address a, PortNum p);
  void startListen();
  void stopListen();
  //AudioVector retrieveStream();
  int sendStream(ClientToken t, ClientTID tid, AudioVector& v);
private:
  void initSocket();
  void bindSocket();
  void initServerAddress(Address a, PortNum p);
  //void setToken(ClientToken t);
  void listen();
  int send(const void* buff, size_t s);
  int receive(void* buff, size_t s);
  int receiveWithTimeout(fd_set* fds, void* buff, size_t s);
  // ********************
  bool listening();
  //void resetTIDs();
  // ********************
  SocketFD mySockFD;
  struct sockaddr_in myAddrss, servAddrss;
  socklen_t myAddrssLen, servAddrssLen;
  struct timeval myTimeout;
  // ********************
  UDPDatagram myInDatagram, myOutDatagram;
  StreamQueue& myStreamQueue;
  //ClientTID myLastRqst; // Last TID used for requests to server...
  //ClientToken myToken;
  // ********************
  bool active;
  std::mutex myMutex;
  std::thread myThread;
};

//void clearVector(AudioVector& v);

#endif
