#ifndef STREAM_PAYLOAD_H
#define STREAM_PAYLOAD_H

#include <cstdint>
#include "AudioVector.h"
#include "Payload.h"
#include "commons.h"
#include "types.h"

class StreamPayload: public Payload {
public:
  typedef uint8_t Mode;
  typedef uint32_t StreamSize;
  typedef uint8_t Flag;
  // ********************
  StreamPayload(const MaxSize ms);
  ~StreamPayload();
  // ********************
  ClientToken token() const;
  ClientTID tid() const;
  StreamSize streamSize() const;
  AudioVector streamCopy() const;
  Flag flag() const;
  // ********************
  void buildAudioStream(const ClientToken t, const ClientTID tid, const AudioVector& v, const Flag f = STANDARD_STREAM);
};

#endif
