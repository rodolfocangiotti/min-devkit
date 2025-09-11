#include <cassert>
#include <iostream>
#include "TCPConnection.h"
#include "commons.h"

#if defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#endif

TCPConnectionException::TCPConnectionException(const std::string e) noexcept:
  error(e) {
  error.insert(0, "[ERROR] TCPConnectionException: ");
  // TODO
}

TCPConnectionException::~TCPConnectionException() {
  // TODO
}

const char* TCPConnectionException::what() const noexcept {
  return error.c_str();
}

TCPConnection::TCPConnection():
  mySockFD(0),
  myAddrss(), servAddrss(),
  myAddrssLen(0), servAddrssLen(0),
  myPayload(TCP_BUFFER_SIZE),
  connected(false) {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Constructing TCPConnection class..." << '\n';
#endif
}

TCPConnection::~TCPConnection() {
#if defined(__APPLE__) || defined(__linux__)
  close(mySockFD);
#elif defined(_WIN32)
  closesocket(mySockFD);
  WSACleanup();
#endif
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Destructing TCPConnection class..." << '\n';
#endif
}

void TCPConnection::initSocket() {
#if defined(DEBUG) && VERBOSENESS > 1
  std::cout << "[DEBUG] Initializing TCPConnection socket..." << '\n';
#endif
#ifdef _WIN32
  WSAData wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    //perror("WSAStartup()"); // TODO Check if on Windows the perror() function is supported...
    std::cerr << "WSAStartup failed with error " << WSAGetLastError() << '\n';
    throw TCPConnectionException("Winsock initialization failed.");
  }
#endif
  if ((mySockFD = socket(PF_INET, SOCK_STREAM, 0)) < 0) { // Create socket file descriptor for UDP protocol...
    perror("socket()");
#ifdef _WIN32
    WSACleanup();
#endif
    throw TCPConnectionException("Socket creation failed.");
  }
  int enable = 1;
  if (setsockopt(mySockFD, SOL_SOCKET, SO_REUSEADDR, (const char*) &enable, sizeof enable) < 0) { // TODO Check if SO_REUSEADDR is okay...
    perror("setsockopt()");
#ifdef _WIN32
    WSACleanup();
#endif
    throw TCPConnectionException("Socket option setting failed.");
  }
}

void TCPConnection::bindSocket() {
  myAddrss.sin_family = AF_INET;  // Fill TCPConnection information...
  myAddrss.sin_addr.s_addr = INADDR_ANY;
  myAddrss.sin_port = 0;  // Let the system choose one available port...
  myAddrssLen = sizeof myAddrss;
  if (bind(mySockFD, (const struct sockaddr*) &myAddrss, myAddrssLen) < 0) {  // Bind the socket with the TCPConnection address...
    perror("bind()");
#ifdef _WIN32
    WSACleanup();
#endif
    throw TCPConnectionException("Socket bind failed.");
  }
#if defined(DEBUG) && VERBOSENESS > 1
  if (getsockname(mySockFD, (struct sockaddr*) &myAddrss, &myAddrssLen) < 0) {
    perror("getsockname()");
  }
  std::cout << "[DEBUG] Socket bound on " << inet_ntoa(myAddrss.sin_addr) << ":" << ntohs(myAddrss.sin_port) << "..." << '\n'; // TODO Convert to warning or info?
#endif
}

void TCPConnection::initServerAddress(Address a, PortNum p) {
  servAddrss.sin_family = AF_INET;  // Filling server information...
  servAddrss.sin_addr.s_addr = inet_addr(a.c_str());
  servAddrss.sin_port = htons(p);
  servAddrssLen = sizeof servAddrss;
}

void TCPConnection::configure(Address servIP, PortNum servPort) {
  initSocket();
  bindSocket();
  initServerAddress(servIP, servPort);
}

void TCPConnection::connect() {
  if (!(connected)) {
    if (::connect(mySockFD, (const struct sockaddr*) &servAddrss, servAddrssLen) < 0) { // XXX Probable name conflict...
      perror("connect()");
      throw TCPConnectionException("Connection to server failed.");
    }
    connected = true;
#if defined(DEBUG) && VERBOSENESS > 1
    if (getsockname(mySockFD, (struct sockaddr*) &myAddrss, &myAddrssLen) < 0) {
      perror("getsockname()");
    }
    std::cout << "[DEBUG] Connecting to server from " << inet_ntoa(myAddrss.sin_addr) << ":" << ntohs(myAddrss.sin_port) << "..." << '\n';
#endif
  }
}

ClientToken TCPConnection::requestEntry() {
  myPayload.buildEntryRequest();
  send(myPayload.rawBuffer(), myPayload.size());
  receive(myPayload.pointWritableBuffer(), TCP_BUFFER_SIZE);
  //assert(myPayload.header() == OKAY);
  if (myPayload.header() != OKAY) {
    throw TCPConnectionException("Entry request failed.");
  }
#if defined(DEBUG) && VERBOSENESS > 0
  std::cout << "[DEBUG] Token received!" << '\n';
#endif
  return myPayload.token();
}

void TCPConnection::requestExit(ClientToken t) {
  myPayload.buildExitRequest(t);
  send(myPayload.rawBuffer(), myPayload.size());
  receive(myPayload.pointWritableBuffer(), TCP_BUFFER_SIZE);
  //assert(myPayload.header() == OKAY);
  if (myPayload.header() != OKAY) {
    throw TCPConnectionException("Exit request failed.");
  }
#if defined(DEBUG) && VERBOSENESS > 0
  std::cout << "[DEBUG] Token released!" << '\n';
#endif
}

void TCPConnection::disconnect() {
  if (connected) {
#if defined(__APPLE__) || defined(__linux__)
    shutdown(mySockFD, SHUT_RDWR);
    close(mySockFD);
#elif defined(_WIN32)
    shutdown(mySockFD, SD_BOTH);
    closesocket(mySockFD);
    WSACleanup();
#endif
    connected = false;
  }
}

void TCPConnection::send(const void* buff, size_t s) {
  if (::send(mySockFD, (const char*) buff, s, 0) < 0) {
    perror("write()");
  }
}

void TCPConnection::receive(void* buff, size_t s) {
  if (recv(mySockFD, (char*) buff, s, 0) < 0) {
    perror("read()");
  }
}
