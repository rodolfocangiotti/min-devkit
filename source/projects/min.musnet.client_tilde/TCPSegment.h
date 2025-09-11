#ifndef TCP_SEGMENT_H
#define TCP_SEGMENT_H

#include <cstdint>
#include "Payload.h"
#include "types.h"

class TCPSegment: public Payload {
public:
  TCPSegment(MaxSize ms);
  ~TCPSegment();
  // ********************
  ClientToken token() const;
  // ********************
  void buildEntryRequest();
  void buildEntryResponse(ClientToken t);
  void buildExitRequest(ClientToken t);
  void buildExitResponse();
};

#endif
