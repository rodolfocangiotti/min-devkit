#ifndef UDP_DATAGRAM_H
#define UDP_DATAGRAM_H

#include <cstdint>
#include "StreamPayload.h"
#include "commons.h"
#include "types.h"

class UDPDatagram: public StreamPayload {
public:
  typedef uint8_t Mode;
  typedef uint16_t StreamSize;
  // ********************
  UDPDatagram(const MaxSize ms);
  ~UDPDatagram();
  // ********************
  ClientTID parentTID() const;
  // ********************
  void setParentTID(const ClientTID tid);
private:
  ClientTID myParentTID;
};

#endif
