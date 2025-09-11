#include <cassert>
#include <cmath>
#include <iostream>
#include "UDPConnection.h"
#include "prettyprint.h"

#if defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#endif

UDPConnection::UDPConnection(StreamQueue& sq):
  mySockFD(0),
  myAddrss(), servAddrss(),
  myAddrssLen(0), servAddrssLen(0),
  myInDatagram(UDP_BUFFER_SIZE), myOutDatagram(UDP_BUFFER_SIZE),
  myStreamQueue(sq),
  //myLastRqst(0),
  //myToken(0),
  active(false),
  myMutex(), myThread() {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Constructing UDPConnection class..." << '\n';
#endif
}

UDPConnection::~UDPConnection() {
#if defined(__APPLE__) || defined(__linux__)
  close(mySockFD);
#elif defined(_WIN32)
  closesocket(mySockFD);
  WSACleanup();
#endif
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Destructing UDPConnection class..." << '\n';
#endif
}

void UDPConnection::initSocket() {
#if defined(DEBUG) && VERBOSENESS > 1
  std::cout << "[DEBUG] Initializing UDPConnection socket..." << '\n';
#endif
#ifdef _WIN32
  WSAData wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    perror("WSAStartup()");
    std::cerr << "WSAStartup failed with error " << WSAGetLastError() << '\n';
    exit(EXIT_FAILURE);
  }
#endif
  if ((mySockFD = socket(PF_INET, SOCK_DGRAM, 0)) < 0) { // Create socket file descriptor for UDP protocol...
#ifdef _WIN32
    WSACleanup();
#endif
    perror("socket()");
    exit(EXIT_FAILURE);
    //throw UDPConnectionException("Socket creation failed.");
  }
  int enable = 1;
  if (setsockopt(mySockFD, SOL_SOCKET, SO_REUSEADDR, (const char*) &enable, sizeof enable) < 0) { // TODO Check if SO_REUSEADDR is okay...
#ifdef _WIN32
    WSACleanup();
#endif
    perror("setsockopt()");
    exit(EXIT_FAILURE);
    //throw UDPConnectionException("Socket options setting failed.");
  }
}

void UDPConnection::bindSocket() {
  myAddrss.sin_family = AF_INET;  // Fill UDPConnection information...
  myAddrss.sin_addr.s_addr = INADDR_ANY;
  myAddrss.sin_port = 0;  // Let the system choose one available port...
  myAddrssLen = sizeof myAddrss;
  if (bind(mySockFD, (const struct sockaddr*) &myAddrss, myAddrssLen) < 0) {  // Bind the socket with the UDPConnection address...
#ifdef _WIN32
    WSACleanup();
#endif
    perror("bind()");
    exit(EXIT_FAILURE);
    //throw UDPConnectionException("Socket bind failed.");
  }
#if defined(DEBUG) && VERBOSENESS > 1
  if (getsockname(mySockFD, (struct sockaddr*) &myAddrss, &myAddrssLen) < 0) {
#ifdef _WIN32
    WSACleanup();
#endif
    perror("getsockname()");
    exit(EXIT_FAILURE);
  }
  std::cout << "[DEBUG] Socket bound on " << inet_ntoa(myAddrss.sin_addr) << ":" << ntohs(myAddrss.sin_port) << "..." << '\n'; // TODO Convert to warning or info?
#endif
}

void UDPConnection::initServerAddress(Address a, PortNum p) {
  servAddrss.sin_family = AF_INET;  // Filling server information...
  servAddrss.sin_addr.s_addr = inet_addr(a.c_str());
  servAddrss.sin_port = htons(p);
  servAddrssLen = sizeof servAddrss;
}

/*
void UDPConnection::setToken(ClientToken t) {
  myToken = t;
}
*/

void UDPConnection::configure(Address a, PortNum p) {
  initSocket();
  bindSocket();
  initServerAddress(a, p);
  //setToken(t)
}

void UDPConnection::listen() {
  // Define a set of file descriptors...
  fd_set currSet;
  FD_ZERO(&currSet);
  FD_SET(mySockFD, &currSet);
  fd_set nextSet = currSet;

  //resetTIDs(); // TODO Check if this function should be placed somewhere else in the code!

  while (listening()) {
    // Receive datagram from client...
    int res = receiveWithTimeout(&currSet, myInDatagram.pointWritableBuffer(), UDP_BUFFER_SIZE);
    if (!(res > 0)) {
      if (res < 0) {
        perror("receiveWithTimeout()");
        std::cerr << RED << "[ERROR] Error receiving datagram!" << RESET << '\n';
      } else {  // res is equal to 0...
#if defined(DEBUG) && VERBOSENESS > 1
        std::cout << "[DEBUG] UDP timeout reached!" << '\n';
#endif
      }
      currSet = nextSet;
      continue;
    }

#if defined(DEBUG) && VERBOSENESS > 2
    std::cout << "[DEBUG] Datagram received!" << '\n';
#endif
    // TODO CHECK THIS LITTLE CODE BLOCK!
    AudioVector fromServ = myInDatagram.streamCopy();
    ClientToken t = myInDatagram.token();
    ClientTID respTID = myInDatagram.tid();
    assert(fromServ.size() == AUDIO_VECTOR_SIZE * NUM_CHANNELS);
    myStreamQueue.insertVector(t, respTID, fromServ);
    currSet = nextSet;
  }
}

void UDPConnection::startListen() {
  std::lock_guard<std::mutex> l(myMutex);
  if (!(active)) {
    myThread = std::thread(&UDPConnection::listen, this);
    active = true;
  }
}

void UDPConnection::stopListen() {
  std::cout << "Launching stopListen()..." << '\n';
  bool join = false;
  {
    std::lock_guard<std::mutex> l(myMutex);
    if (active) {
      active = false;
      join = true;
    }
  }
  if (join) {
    if (myThread.joinable()) {
      myThread.join();
    }
  }
  std::cout << "Listen stopped!" << '\n';
}

/*
AudioVector UDPConnection::retrieveStream() {
  ClientTID tid = getRequestTID() - 1;
  return myBuff.readVector(tid);
}
*/

int UDPConnection::sendStream(ClientToken t, ClientTID tid, AudioVector &v) {
  assert(v.size() == AUDIO_VECTOR_SIZE * NUM_CHANNELS);
  myOutDatagram.buildAudioStream(v, t, tid);
  return send(myOutDatagram.rawBuffer(), UDP_BUFFER_SIZE);
}

int UDPConnection::send(const void* buff, size_t s) {
  return sendto(mySockFD, (const char*) buff, s, 0, (const struct sockaddr*) &servAddrss, servAddrssLen);
}

int UDPConnection::receive(void* buff, size_t s) {
  return recvfrom(mySockFD, (char*) buff, s, 0, (struct sockaddr*) &servAddrss, &servAddrssLen);
}

int UDPConnection::receiveWithTimeout(fd_set* fds, void* buff, size_t s) {
  myTimeout = {1, 0}; // On Linux, select() overwirte the timeval structure with the waiting time...
  // Due to this, before each call, the structure must be re-initialised...
  int descrAmount = select(mySockFD + 1, fds, NULL, NULL, &myTimeout);
  if (descrAmount > 0) {
    assert(FD_ISSET(mySockFD, fds));
    assert(descrAmount == 1); // Only one socket is listening, so only one descriptor should be modified...
    return receive(buff, s);
  }
  if (descrAmount < 0) {
    std::cout << RED << "[ERROR] Unknown error!" << RESET << '\n';
    return descrAmount;
  }
#if defined(DEBUG) && VERBOSENESS > 1
  perror("select()");
  std::cout << "[DEBUG] Timeout reached!" << '\n';
#endif
  return 0;  // Return undefined error...
}

bool UDPConnection::listening() {
  std::lock_guard<std::mutex> l(myMutex);
  return active;
}

/*
ClientTID UDPConnection::getRequestTID() {
  std::lock_guard<std::mutex> l(myMutex);
  return myReqstTID;
}

ClientTID UDPConnection::getNewRequestTID() {
  std::lock_guard<std::mutex> l(myMutex);
  return ++myReqstTID;
}

void UDPConnection::resetTID() {
  std::lock_guard<std::mutex> l(myMutex);
  myReqstTID = 0;
  myLastRespTID = 0;
}

void clearVector(AudioVector& v) {
  for (int i = 0; i < v.size(); i++) {
    v[i] = 0.0;
  }
}
*/
