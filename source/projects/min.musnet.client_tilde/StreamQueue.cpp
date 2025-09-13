#include <cassert>
#include <iostream>
#include "StreamQueue.h"
#include "commons.h"
#include "prettyprint.h"

bool comp(const StreamVector& a, const StreamVector& b) {
  return a.tid() < b.tid();
}

StreamQueue::StreamQueue():
  myCanReadQueue(false),
  myQueue(),
  myMutex(),
  myMaxLen(MAX_QUEUE_LENGTH) {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Constructing StreamQueue class..." << '\n';
#endif
}

StreamQueue::~StreamQueue() {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Destructing StreamQueue class..." << '\n';
#endif
}

//void StreamQueue::insertVector(ClientToken t, ClientTID tid, AudioVector& v) {
void StreamQueue::insertVector(ClientToken t, StreamVector sv) { // Voglio che il vettore sia copiato...
    // niente referenza...
    /*
     2025/09/12: con questo for si elimina la ridondanza. Da commentare.
    for (std::list<ClientTID>::iterator it = myHisto.begin(); it != myHisto.end(); it++) {
        if (*it == sv.tid()) {
            // This TID has been already received...
            std::cout << "Dupe found. Exiting..." << '\n';
            return;
        }
    }
     */
//  StreamVector sv;
//  sv.setTID(tid);
//  sv.setAudioVector(v);
//  sv.addReadPermission(t);
  {
    // Critical section...
    std::lock_guard<std::mutex> l(myMutex);
//    if (myQueue.size() >= myMaxLen) {
//      myQueue.clear();
//    }
    myQueue.push_back(sv);
//    myQueue.sort(comp);
//    removeDuplicates();
//
//    std::list<StreamVector>::iterator it = myQueue.begin();
//    StreamVector* prev;
//    while (it != myQueue.end()) {
//      if (prev == nullptr) {
//        prev = &(*it);
//        it++;
//        continue;
//      }
//      if (it->tid() == prev->tid()) {
//        std::list<StreamVector>::iterator toRemove = it;
//        it++;
//        myQueue.erase(toRemove);
//        std::cout << "DEBUG: REMOVING DUPLICATE ENTRY\n";
//      } else {
//        prev = &(*it);
//        it++;
//      }
//    }
  }
    myHisto.push_back(sv.tid());
    while (myHisto.size() > 16) {
        myHisto.pop_front();
    }
}

AudioVector StreamQueue::retrieveVector(ClientToken t, bool readAlways) {
  AudioVector v(AUDIO_VECTOR_SIZE * NUM_CHANNELS, 0.0);
  StreamVector sv;
  bool retrieved = false;
  // XXX Main audio thread should not use mutex, locker, etc. because time to lock a thread is unpredictable and so this could in the worst situation generate audio stream interruptions...
  {
    // Critical section...
    std::lock_guard<std::mutex> l(myMutex);
    if (!(myCanReadQueue)) { // Add limits to queue read in order to reduce artefact frequency...
//      if (myQueue.size() >= MIN_QUEUE_LENGTH) {
      if (myQueue.size() >= myMaxLen / 2) {
        myCanReadQueue = true;
      }
    } else {
      assert(myCanReadQueue);
      if (myQueue.empty()) {
        myCanReadQueue = false;
      }
    }
    if (myCanReadQueue || readAlways) {  // Read queue if the above policies allow this...
        myQueue.sort(comp);
        while (myQueue.size() > myMaxLen) {
          myQueue.pop_front();
        }
      sv = myQueue.front();
      retrieved = true;
      myQueue.pop_front();
#if defined(DEBUG) && VERBOSENESS > 2
      std::cout << "Current queue size: " << myQueue.size() << '\n';
#endif
    }
  }
  if (!(retrieved)) {
    std::cerr << RED << "[ERROR] No readable vectors on queue (queue is empty or haven't reached the reading threshold)!" << RESET << '\n';
    return v;
  }
  v = sv.readVector(t);
  return v;
}

int StreamQueue::retrieveVector(StreamVector& res) {
    // Critical section...
    std::lock_guard<std::mutex> l(myMutex); // XXX MUTEX WAS MISSING, BUT ON MACOS NOTHING HAPPENED...
    // ON LINUX, INSTEAD, SEGMENTATION FAULT AFTER SOME SECS...
    if (myQueue.size()) {
        res = myQueue.front();
        myQueue.pop_front();
        return 0;
    }
    return -1;
}

void StreamQueue::setMaxLength(int elems) {
    myMaxLen = elems;
}
