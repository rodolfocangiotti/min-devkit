#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <thread>
#include "AudioVector.h"
#include "StreamQueue.h"
#include "TCPSegment.h"
#include "types.h"

#if defined(__APPLE__) || defined(__linux__)
#include <arpa/inet.h>
#elif defined(_WIN32)
#include <winsock2.h>
#include <WS2tcpip.h>
#endif

class TCPConnectionException: public std::exception {
public:
  TCPConnectionException(const std::string e) noexcept;
  virtual ~TCPConnectionException();
  virtual const char* what() const noexcept;
private:
  std::string error;
};

class TCPConnection {
public:
  TCPConnection(StreamQueue& in_queue, StreamQueue& out_queue);
  ~TCPConnection();
  // ********************
  void configure(Address servAddrss, PortNum servPort);
  void startListen();
  void stopListen();
  int sendStream(ClientToken t, ClientTID tid, AudioVector& v, Flag f);
  void connect();
  void disconnect();
  ClientToken requestEntry();
  void requestExit(ClientToken t);
  void set_mode(Flag f);
  Flag get_mode() const;
private:
  void initSocket();
  void bindSocket();
  void initServerAddress(Address servAddrss, PortNum servPort);
  void listen();
  void asyncSendStream();
  int send(const void* buff, size_t s);
  int receive(void* buff, size_t s);
  int receiveWithTimeout(fd_set* fds, void* buff, size_t s);
  bool listening();
  SocketFD mySockFD;
  struct sockaddr_in myAddrss, servAddrss;
  struct timeval myTimeout;
  socklen_t myAddrssLen, servAddrssLen;
  TCPSegment myDefaultSegment, myInSegment, myOutSegment;
  StreamQueue& myInStreamQueue;
  StreamQueue& myOutStreamQueue;
  std::atomic<bool> active;
  bool connected;
  std::mutex myMutex;
  std::thread myInThread, myOutThread;
  Flag _mode;
};

#endif
