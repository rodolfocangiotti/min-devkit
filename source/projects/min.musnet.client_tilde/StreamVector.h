#ifndef STREAM_VECTOR_H
#define STREAM_VECTOR_H

#include <map>
#include "AudioVector.h"
#include "commons.h"
#include "types.h"

class StreamVector {
public:
  StreamVector();
  ~StreamVector();
  // ********************
  ClientTID tid() const;
  ClientToken owner() const;
  // ********************
  void setAudioVector(const AudioVector& v);
  void setTID(ClientTID tid);
  void setOwner(ClientToken t);
  void setCreationTimestamp();
  SystemTime getCreationTimestamp() const;
  void set_flag(Flag f);
  Flag get_flag() const;
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
  SystemTime creation_time;
  Flag _flag;
};

#endif
