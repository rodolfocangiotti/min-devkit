#include <cassert>
#include <iostream>
#include "StreamQueue.h"
#include "commons.h"
#include "prettyprint.h"

bool comp(const StreamVector& a, const StreamVector& b) {
  return a.tid() < b.tid();
}

StreamQueue::StreamQueue():
  myReadQueue(false),
  myQueue(),
  myMutex() {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Constructing StreamQueue class..." << '\n';
#endif
}

StreamQueue::~StreamQueue() {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Destructing StreamQueue class..." << '\n';
#endif
}

void StreamQueue::insertVector(ClientToken t, ClientTID tid, AudioVector& v) {
  StreamVector sv;
  sv.setTID(tid);
  sv.setAudioVector(v);
  sv.addReadPermission(t);
  {
    // Critical section...
    std::lock_guard<std::mutex> l(myMutex);
    myQueue.push_back(sv);
    myQueue.sort(comp);
  }
}

AudioVector StreamQueue::retrieveVector(ClientToken t) {
  AudioVector v(AUDIO_VECTOR_SIZE * NUM_CHANNELS, 0.0);
  StreamVector sv;
  bool retrieved = false;
  // XXX Main audio thread should not use mutex, locker, etc. because time to lock a thread is unpredictable and so this could in the worst situation generate audio stream interruptions...
  {
    // Critical section...
    std::lock_guard<std::mutex> l(myMutex);
    if (!(myReadQueue)) { // Add limits to queue read in order to reduce artefact frequency...
      if (myQueue.size() >= MIN_QUEUE_LENGTH) {
        myReadQueue = true;
      }
    } else {
      assert(myReadQueue);
      if (myQueue.empty()) {
        myReadQueue = false;
      }
    }
    if (myReadQueue) {  // Read queue if the above policies allow this...
      sv = myQueue.front();
      retrieved = true;
      myQueue.pop_front();
      std::cout << "Current queue size: " << myQueue.size() << '\n';
    }
  }
  if (!(retrieved)) {
    std::cerr << RED << "[ERROR] No readable vectors on queue (queue is empty or haven't reached the reading threshold)!" << RESET << '\n';
    return v;
  }
  v = sv.readVector(t);
  return v;
}
