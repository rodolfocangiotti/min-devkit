#ifndef STREAM_QUEUE_H
#define STREAM_QUEUE_H

#include <mutex>
#include <list>
#include "StreamVector.h"
#include "types.h"

bool comp(const StreamVector& a, const StreamVector& b);

class StreamQueue {
public:
  StreamQueue();
  ~StreamQueue();
  void insertVector(ClientToken t, ClientTID wtid, AudioVector& v);
  AudioVector retrieveVector(ClientToken t);
private:
  bool myReadQueue;
  std::mutex myMutex;
  std::list<StreamVector> myQueue;
};

#endif
