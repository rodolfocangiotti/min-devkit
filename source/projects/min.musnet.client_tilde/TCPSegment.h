#ifndef TCP_SEGMENT_H
#define TCP_SEGMENT_H

#include <cstdint>
#include "StreamPayload.h"
#include "types.h"

class TCPSegment: public StreamPayload {
public:
  TCPSegment(const MaxSize ms);
  ~TCPSegment();
  // ********************
  void buildEntryRequest();
  void buildEntryResponse(const ClientToken t);
  void buildExitRequest(const ClientToken t);
  void buildExitResponse();
};

#endif
