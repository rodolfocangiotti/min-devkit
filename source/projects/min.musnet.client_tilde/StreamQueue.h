#ifndef STREAM_QUEUE_H
#define STREAM_QUEUE_H

#include <deque>
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
    int retrieveVector(StreamVector& res, bool debug);
    std::vector<StreamVector> retrieveAllVectors();
    void setMaxLength(int elems);
    std::mutex& referMutex();
    void clear();
private:
  bool myCanReadQueue;
  std::mutex myMutex;
  std::deque<StreamVector> myQueue;
  std::list<ClientTID> myHisto;
  int myMaxLen;
};

#endif
