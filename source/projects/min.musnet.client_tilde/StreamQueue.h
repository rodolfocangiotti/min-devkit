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
//  void insertVector(ClientToken t, ClientTID wtid, AudioVector& v);
    void insertVector(ClientToken t, StreamVector sv);
  AudioVector retrieveVector(ClientToken t, bool readAlways = false);
    int retrieveVector(StreamVector& res);
    void setMaxLength(int elems);
private:
  bool myCanReadQueue;
  std::mutex myMutex;
  std::list<StreamVector> myQueue;
  std::list<ClientTID> myHisto;
  int myMaxLen;
};

#endif
