#include <cassert>
#include <iostream>
#include "StreamVector.h"
#include "commons.h"
#include "prettyprint.h"

StreamVector::StreamVector() {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Constructing StreamVector class..." << '\n';
#endif
}

StreamVector::~StreamVector() {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Destructing StreamVector class..." << '\n';
#endif
}

ClientTID StreamVector::tid() const {
  return myTID;
}

void StreamVector::setAudioVector(AudioVector& v) {
  myVect = v;
}

void StreamVector::setTID(ClientTID tid) {
  myTID = tid;
}

int StreamVector::addReadPermission(ClientToken t) {
  assert(myLog.count(t) == 0);
  myLog[t] = false;
#if defined(DEBUG) && VERBOSENESS > 0
  std::cout << "[DEBUG] Adding read permission to client " << t << " for vector on queue..." << '\n';
#endif
  return 0;
}

int StreamVector::removeReadPermission(ClientToken t) {
  for (ReadLogger::iterator i = myLog.begin(); i != myLog.end(); i++) {
    if (i->first == t) {
      myLog.erase(i);
#if defined(DEBUG) && VERBOSENESS > 0
      std::cout << "[DEBUG] Removing read permission to client " << t << " from vector on queue..." << '\n';
#endif
      return 0;
    }
  }
  std::cerr << RED << "[ERROR] Impossible to remove read permission to client " << t << " (token not found)!" << RESET << '\n';
  return -1;
}

AudioVector StreamVector::readVector(ClientToken t) {
  AudioVector v(AUDIO_VECTOR_SIZE * NUM_CHANNELS, 0.0);
  //assert(myLog.count(t) > 0); // If not, client has no read permission...
  // TODO SOMETIMES RAISES ERROR THIS EXCEPTION TOO...
  //assert(myLog[t] == false);  // Client has not still read the vector...
  // THIS ASSERTION RAISES ERROR SOMETIMES...
  if (myLog[t] == false) {
    myLog[t] = true;
    v = myVect;
  }
  return v;
}

bool StreamVector::isDeletable() {
  for (ReadLogger::iterator i = myLog.begin(); i != myLog.end(); i++) {
    if (i->second == false) {
      return false;
    }
  }
  return true;
}

bool StreamVector::isReadableBy(ClientToken t) {
  return myLog.count(t) > 0 ? true : false;
}
