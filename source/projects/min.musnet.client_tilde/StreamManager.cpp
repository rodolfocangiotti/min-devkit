#include <iostream>
#include "StreamManager.h"

//#define HOSTADDR "192.168.1.2"  // MacBook on Wi-Fi LAN...
#define TCPPORT 50000
#define UDPPORT 50001

StreamManager::StreamManager():
  connected(false),
  myToken(0),
  myReqstTID(0), myRespTID(0),
  myQueue(),
  myTCPServ(),
  myUDPServ(myQueue) {
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
      myTCPServ.configure(myIP.c_str(), TCPPORT); // TODO Improve exception management...
      myTCPServ.connect();
      myToken = myTCPServ.requestEntry();
      myTCPServ.disconnect();
      myUDPServ.configure(myIP.c_str(), UDPPORT);
      myUDPServ.startListen();
    } catch (TCPConnectionException& e) {
      return -1;
    }
    connected = true;
  }
  return 0;
}

int StreamManager::disconnect() {
  if (connected) {
    try {
      myTCPServ.configure(myIP.c_str(), TCPPORT);
      myTCPServ.connect();
      myTCPServ.requestExit(myToken);
      myTCPServ.disconnect();
      myUDPServ.stopListen();
    } catch (TCPConnectionException& e) {
      return -1;
    }
    connected = false;
  }
  return 0;
}

void StreamManager::sendStream(AudioVector& v) {
  if (connected) {
    ClientTID reqstTID = ++myReqstTID;
    myUDPServ.sendStream(myToken, reqstTID, v);
  }
}

AudioVector StreamManager::retrieveStream() {
  if (connected) {
    ClientTID respTID = ++myRespTID;  // TODO
    return myQueue.retrieveVector(myToken);
  } else {
    return AudioVector(AUDIO_VECTOR_SIZE * NUM_CHANNELS, 0.0);
  }
}
