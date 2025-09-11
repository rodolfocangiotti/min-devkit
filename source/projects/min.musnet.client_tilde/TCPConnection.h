#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <thread>
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
  TCPConnection();
  ~TCPConnection();
  // ********************
  void configure(Address servAddrss, PortNum servPort);
  void connect();
  void disconnect();
  ClientToken requestEntry();
  void requestExit(ClientToken t);
private:
  void initSocket();
  void bindSocket();
  void initServerAddress(Address servAddrss, PortNum servPort);
  void send(const void* buff, size_t s);
  void receive(void* buff, size_t s);
  SocketFD mySockFD;
  struct sockaddr_in myAddrss, servAddrss;
  socklen_t myAddrssLen, servAddrssLen;
  TCPSegment myPayload;
  bool connected;
};

#endif
