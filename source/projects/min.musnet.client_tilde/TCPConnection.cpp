#include <cassert>
#include <iostream>
#include "TCPConnection.h"
#include "commons.h"
#include "prettyprint.h"

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

TCPConnection::TCPConnection(StreamQueue& in_queue, StreamQueue& out_queue):
    mySockFD(0),
    myAddrss(), servAddrss(),
    myAddrssLen(0), servAddrssLen(0),
    myDefaultSegment(TCP_BUFFER_SIZE), myInSegment(UDP_BUFFER_SIZE), myOutSegment(UDP_BUFFER_SIZE),
    // It's okay to have three different TCP segment instances: one for the operations on the main thread,
    // one for the send thread and another for the receive thread.
    myInStreamQueue(in_queue), myOutStreamQueue(out_queue),
    active(false),
    connected(false),
    myMutex(),
    myInThread(), myOutThread() {
#if defined(DEBUG) && VERBOSENESS > 2
    std::cout << "[DEBUG] Constructing TCPConnection class..." << '\n';
#endif
}

TCPConnection::~TCPConnection() {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Destructing TCPConnection class..." << '\n';
#endif
  disconnect();
  stopListen();
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
  if ((mySockFD = socket(PF_INET, SOCK_STREAM, 0)) < 0) { // Create socket file descriptor for TCP protocol...
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


void TCPConnection::listen() {
  std::cout << "listen() started with ID " << std::this_thread::get_id() << std::endl;
  // Define a set of file descriptors...
  fd_set currSet;
  FD_ZERO(&currSet);
  FD_SET(mySockFD, &currSet);
  fd_set nextSet = currSet;

  //resetTIDs(); // TODO Check if this function should be placed somewhere else in the code!

  while (listening()) {
    // std::cout << "Thread check: " << std::this_thread::get_id() << std::endl;
    // Receive datagram from client...
    size_t sz = UDP_BUFFER_SIZE;
    Flag m = get_mode();
    if (m == SEND_ONLY_STREAM) {
        sz = 10;
    }
    int res = receiveWithTimeout(&currSet, myInSegment.pointWritableBuffer(), sz);
    if (res <= 0) {
      if (res < 0) {
        // perror("receiveWithTimeout()");
        std::cerr << RED << "[ERROR] Error receiving segment!" << RESET << '\n';
      } else {  // res is equal to 0...
#if defined(DEBUG) && VERBOSENESS > 1
        std::cout << "[DEBUG] TCP timeout reached!" << '\n';
#endif
      }
      currSet = nextSet;
      continue;
    }

    // Profiler::add_record(myInDatagram.token(), myInDatagram.tid(), Profiler::OperationID::UDP_RECEIPT);
#if defined(DEBUG) && VERBOSENESS > 2
    std::cout << "[DEBUG] Datagram received!" << '\n';
#endif
    // TODO CHECK THIS LITTLE CODE BLOCK!
    if (myInSegment.header() == AUDIO_STREAM_DATA) {
      const TCPSegment::Size sz = myInSegment.size();   // TODO Why is it always 0?
      const TCPSegment::StreamSize ssz = myInSegment.streamSize();
      const AudioVector fromServ = myInSegment.streamCopy();
      const TCPSegment::Flag flag = myInSegment.flag();
      if (flag == SEND_ONLY_STREAM) {
        assert(fromServ.size() == 0);
      } else {
        assert(fromServ.size() == AUDIO_VECTOR_SIZE * NUM_CHANNELS);
      }
      ClientToken t = myInSegment.token();
      ClientTID respTID = myInSegment.tid();
      StreamVector sv;
      sv.addReadPermission(t);
      sv.setAudioVector(fromServ);
      sv.setTID(respTID);
      //assert(fromServ.size() == AUDIO_VECTOR_SIZE * NUM_CHANNELS);
      myOutStreamQueue.insertVector(t, sv);
      // Profiler::add_record(t, respTID, Profiler::OperationID::RESPONSE_PENDING);
    } else {
      std::cerr << "Unexpected header (" << static_cast<int>(myInSegment.header()) << ")!" << '\n';
    }
    currSet = nextSet;
  }
}

void TCPConnection::startListen() {
  // std::lock_guard<std::mutex> l(myMutex);
  // if (!(active)) {
  if (!(active.load())) {
    // active = true;
    active.store(true);
    // Threads inside check the variable active to understand if they should run or not...
    // So, set active before starting the threads...
    // TODO Implement a set of intermediate states of the connection.
    // TAKE INTO ACCOUNT CAS OPERATION (PROBABLY NOT NECESSARY HERE, SINCE active IS ASSIGNED ONLY IN THE MAIN THREAD.
    myInThread = std::thread(&TCPConnection::asyncSendStream, this);
    myOutThread = std::thread(&TCPConnection::listen, this);
  }
}

void TCPConnection::stopListen() {
  std::cout << "Launching stopListen()..." << '\n';
  bool join = false;
  {
    // std::lock_guard<std::mutex> l(myMutex);
    // if (active) {
    if (active.load()) {
      // active = false;
      active.store(false);
      join = true;
    }
  }
  if (join) {
    if (myInThread.joinable()) {
      std::cout << "Joining asyncSendStream with ID: " << myInThread.get_id() << std::endl;
      myInThread.join();
    }
    if (myOutThread.joinable()) {
      std::cout << "Joining listen() with ID: " << myOutThread.get_id() << std::endl;
      myOutThread.join();
    }
  }
  std::cout << "Listen stopped!" << '\n';
}

void TCPConnection::asyncSendStream() {
    std::cout << "asyncSendStream() started with ID " << std::this_thread::get_id() << std::endl;
    try {
        StreamVector sv;
        while (listening()) {
            // std::cout << "Thread check: " << std::this_thread::get_id() << std::endl;
            std::vector<StreamVector> from_queue = myInStreamQueue.retrieveAllVectors();
            for (std::vector<StreamVector>::iterator it = from_queue.begin(); it != from_queue.end(); it++) {
                // it is iterating StreamVector...
                // Profiler::add_record(it->owner(), it->tid(), Profiler::OperationID::REQUEST_START);
                ClientToken t = it->owner();
                ClientTID tid = it->tid();
                AudioVector v = it->readVector(t);
                Flag f = it->get_flag();
                if (f == RECEIVE_ONLY_STREAM) {
                    assert(v.size() == 0);
                } else {
                    assert(v.size() == AUDIO_VECTOR_SIZE * NUM_CHANNELS);
                }
                sendStream(t, tid, v, f);
                // Profiler::add_record(t, tid, Profiler::OperationID::UDP_DISPATCH);
            }
        }
    } catch (...) {
        std::cerr << "asyncSendStream: exception catched." << std::endl;
    }
}

int TCPConnection::sendStream(ClientToken t, ClientTID tid, AudioVector &v, Flag f) {
  // assert(v.size() == AUDIO_VECTOR_SIZE * NUM_CHANNELS);
  myOutSegment.buildAudioStream(t, tid, v, f);
  return send(myOutSegment.rawBuffer(), myOutSegment.size());
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

int TCPConnection::send(const void* buff, size_t s) {
  int bytes = ::send(mySockFD, (const char*) buff, s, 0);
  if (bytes <= 0) {
    perror("send()");
  }
  return bytes;
}

int TCPConnection::receive(void* buff, size_t s) {
  /*
  if (recv(mySockFD, (char*) buff, s, 0) < 0) {
    perror("read()");
    return -1;
  }
  return 0;
  */
  /*
  int res = recv(mySockFD, (char*) buff, s, 0);
  if (res < 0) {
    perror("read()");
  }
  return res;
  */
  uint total_bytes = 0;
  uint pointer_shift = 0;
  do {
    int bytes = recv(mySockFD, static_cast<char*>(buff) + pointer_shift, s - total_bytes, 0);
    if (bytes <= 0) {
      if (bytes < 0) {
        perror("receive()");
      }
      return bytes;
    }
    pointer_shift += bytes;
    total_bytes += bytes;
  } while (total_bytes < s);
  assert(total_bytes == s);
  return total_bytes;
}

int TCPConnection::receiveWithTimeout(fd_set* fds, void* buff, size_t s) {
  assert(FD_ISSET(mySockFD, fds));
  myTimeout = {1, 0}; // On Linux, select() overwirte the timeval structure with the waiting time...
  // Due to this, before each call, the structure must be re-initialised...
  int descrAmount = select(mySockFD + 1, fds, NULL, NULL, &myTimeout);
  if (descrAmount > 0) {
    assert(descrAmount == 1); // Only one socket is listening, so only one descriptor should be modified...
    return receive(buff, s);
  } else if (descrAmount < 0) {
    perror("select()");
    // std::cout << RED << "[ERROR] Unknown error!" << RESET << '\n';
    return descrAmount;
  }
#if defined(DEBUG) && VERBOSENESS > 1
  std::cout << "[DEBUG] Timeout reached!" << '\n';
#endif
  return 0;  // Return undefined error...
}

bool TCPConnection::listening() {
  // std::lock_guard<std::mutex> l(myMutex);
  // return active;
  // Convert to atomic variable...
  return active.load();
}


ClientToken TCPConnection::requestEntry() {
  myDefaultSegment.buildEntryRequest();
  
  fd_set currSet;
  FD_ZERO(&currSet);
  FD_SET(mySockFD, &currSet);
  fd_set nextSet = currSet;
  int bytes = send(myDefaultSegment.rawBuffer(), myDefaultSegment.size());
  if (bytes < 0) {
    std::cerr << "[ERROR] Error sending entry segment!" << '\n';
    throw TCPConnectionException("Entry request failed.");
  }
  assert(bytes == 1);
  for (int i {0}; i < 10; i++) {
    bytes = receiveWithTimeout(&currSet, myDefaultSegment.pointWritableBuffer(), 3);
    if (bytes > 0) {
      break;
    }
    if (bytes < 0) {
      std::cerr << "[ERROR] Error receiving entry segment!" << '\n';
    } else {
      // Timeout...
    }
    currSet = nextSet;
  }

  //assert(mySegment.header() == OKAY);
  if ((bytes != 3) || (myDefaultSegment.header() != OKAY_RESPONSE)) {
    throw TCPConnectionException("Entry request failed.");
  }
#if defined(DEBUG) && VERBOSENESS > 0
  std::cout << "[DEBUG] Token received!" << '\n';
#endif
  return myDefaultSegment.token();
}

void TCPConnection::requestExit(ClientToken t) {
  myDefaultSegment.buildExitRequest(t);
    
  fd_set currSet;
  FD_ZERO(&currSet);
  FD_SET(mySockFD, &currSet);
  fd_set nextSet = currSet;
  int bytes = send(myDefaultSegment.rawBuffer(), myDefaultSegment.size());
  if (bytes < 0) {
    std::cerr << "[ERROR] Error sending exit segment!" << '\n';
    throw TCPConnectionException("Exit request failed.");;
  }
  assert(bytes == 3);
  for (int i {0}; i < 10; i++) {
    bytes = receiveWithTimeout(&currSet, myDefaultSegment.pointWritableBuffer(), 1);
    if (bytes > 0) {
      break;
    }
    if (bytes < 0) {
      std::cerr << "[ERROR] Error receiving exit segment!" << '\n';
    } else {
      // Timeout...
    }
    currSet = nextSet;
  }
  
  //assert(mySegment.header() == OKAY);
  if ((bytes != 1) || (myDefaultSegment.header() != OKAY_RESPONSE)) {
    /*
    Buffer b(UDP_BUFFER_SIZE);
    b[0] = myDefaultSegment.header();
    int dbg = 1;
    while (true) {
      std::cerr << dbg " - header: " << static_cast<uint16_t>(b[0]) << std::endl;
      if (b[0] == AUDIO_STREAM_DATA) {
        receive(b.data(), 264);
        receive(b.data(), 1);
        dbg++;
      } else if (b[0] == OKAY_RESPONSE) {
        break;
      }
    }
    */
    throw TCPConnectionException("Exit request failed.");
  }
#if defined(DEBUG) && VERBOSENESS > 0
  std::cout << "[DEBUG] Token released!" << '\n';
#endif
}

void TCPConnection::disconnect() {
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

void TCPConnection::set_mode(Flag f) {
    _mode = f;
}

Flag TCPConnection::get_mode() const {
    return _mode;
}
