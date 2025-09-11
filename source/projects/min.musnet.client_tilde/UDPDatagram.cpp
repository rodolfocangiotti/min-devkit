#include <cassert>
#include <iostream>
#include "UDPDatagram.h"
#include "commons.h"

UDPDatagram::UDPDatagram(MaxSize ms):
  Payload(ms) {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Constructing UDPDatagram class..." << '\n';
#endif
}

UDPDatagram::~UDPDatagram() {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Destructing UDPDatagram class..." << '\n';
#endif
}

ClientToken UDPDatagram::token() const {
  const Byte* bp = &(myBuff[1]);
  const ClientToken* tp = reinterpret_cast<const ClientToken*>(bp);
  return *tp;
}

ClientTID UDPDatagram::tid() const {
  const Byte* bp = &(myBuff[3]);
  const ClientTID* tidp = reinterpret_cast<const ClientTID*>(bp);
  return *tidp;
}

UDPDatagram::StreamSize UDPDatagram::streamSize() const {
  const Byte* bp = &(myBuff[7]);
  const StreamSize* ssp = reinterpret_cast<const StreamSize*>(bp);
  return *ssp;
}

AudioVector UDPDatagram::streamCopy() const { // This method returns a copy of the audio streaming content...
  const Byte* bp = &(myBuff[7]);
  const StreamSize* ssp = reinterpret_cast<const StreamSize*>(bp);
  bp = &(myBuff[9]);
  const AudioSample* asp = reinterpret_cast<const AudioSample*>(bp);
  AudioVector v(*ssp, 0.0);
  for (int i = 0; i < v.size(); i++) {
    v[i] = asp[i];
  }
  return v;
}

void UDPDatagram::buildAudioStream(AudioVector& v, ClientToken t, ClientTID tid) {
  mySize = sizeof (Header) + sizeof (ClientToken) + sizeof (ClientTID) + sizeof (Size) + sizeof (AudioSample) * v.size();
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
  bp = &(myBuff[9]);
  AudioSample* asp = reinterpret_cast<AudioSample*>(bp);
  for (int i = 0; i < v.size(); i++) {
    asp[i] = v[i];
  }
}
