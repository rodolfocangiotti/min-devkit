#include <cassert>
#include <iostream>
#include "StreamPayload.h"
#include "commons.h"

StreamPayload::StreamPayload(const MaxSize ms):
  Payload(ms) {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Constructing StreamPayload class..." << '\n';
#endif
}

StreamPayload::~StreamPayload() {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Destructing StreamPayload class..." << '\n';
#endif
}

ClientToken StreamPayload::token() const {
  const Byte* bp = &(myBuff[1]);
  const ClientToken* tp = reinterpret_cast<const ClientToken*>(bp);
  return *tp;
}

ClientTID StreamPayload::tid() const {
  const Byte* bp = &(myBuff[3]);
  const ClientTID* tidp = reinterpret_cast<const ClientTID*>(bp);
  return *tidp;
}

StreamPayload::StreamSize StreamPayload::streamSize() const {
  const Byte* bp = &(myBuff[7]);
  const StreamSize* ssp = reinterpret_cast<const StreamSize*>(bp);
  return *ssp;
}

AudioVector StreamPayload::streamCopy() const { // This method returns a copy of the audio streaming content...
  const Byte* bp = &(myBuff[7]);
  const StreamSize* ssp = reinterpret_cast<const StreamSize*>(bp);
  bp = &(myBuff[11]);
  const AudioSample* asp = reinterpret_cast<const AudioSample*>(bp);
  AudioVector v(*ssp);
  for (int i = 0; i < v.size(); i++) {
    v[i] = asp[i];
  }
  return v;
}

StreamPayload::Flag StreamPayload::flag() const {
  const Byte* bp = &(myBuff[7]);
  const StreamSize* ssp = reinterpret_cast<const StreamSize*>(bp);
  bp = &(myBuff[11 + sizeof (AudioSample) * *ssp]);
  const Flag* fp = reinterpret_cast<const Flag*>(bp);
  return *fp;
}

void StreamPayload::buildAudioStream(const ClientToken t, const ClientTID tid, const AudioVector& v, const Flag f) {
  mySize = sizeof (Header) + sizeof (ClientToken) + sizeof (ClientTID) + sizeof (StreamSize) + sizeof (AudioSample) * v.size() + sizeof (Flag);
  assert(mySize <= myBuff.size());
  Byte* bp = &(myBuff[0]);
  Header* hp = static_cast<Header*>(bp);
  *hp = AUDIO_STREAM_DATA;
  bp = &(myBuff[1]);
  ClientToken* tp = reinterpret_cast<ClientToken*>(bp);
  *tp = t;
  bp = &(myBuff[3]);
  ClientTID* tidp = reinterpret_cast<ClientTID*>(bp);
  *tidp = tid;
  bp = &(myBuff[7]);
  StreamSize* sp = reinterpret_cast<StreamSize*>(bp);
  *sp = v.size();
  bp = &(myBuff[11]);
  AudioSample* asp = reinterpret_cast<AudioSample*>(bp);
  for (int i = 0; i < v.size(); i++) {
    asp[i] = v[i];
  }
  bp = &(myBuff[11 + sizeof (AudioSample) * v.size()]);
  Flag* fp = reinterpret_cast<Flag*>(bp);
  *fp = f;
}
