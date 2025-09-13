#ifndef STREAM_VECTOR_H
#define STREAM_VECTOR_H

#include <map>
#include "AudioVector.h"
#include "types.h"

class StreamVector {
public:
  StreamVector();
  ~StreamVector();
  // ********************
  ClientTID tid() const;
    ClientToken owner() const;
  // ********************
  void setAudioVector(AudioVector& v);
  void setTID(ClientTID tid);
  void setOwner(ClientToken t);
  // ********************
  int addReadPermission(ClientToken t);
  int removeReadPermission(ClientToken t);
  // ********************
  AudioVector readVector(ClientToken t);
  // ********************
  bool isDeletable();
  bool isReadableBy(ClientToken t);
private:
  typedef std::map<ClientToken, bool> ReadLogger;
  AudioVector myVect;
  ClientTID myTID;
  ReadLogger myLog;
    ClientToken myOwner;
};

#endif
