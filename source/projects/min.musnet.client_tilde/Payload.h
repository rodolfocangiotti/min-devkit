#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <cstdint>
#include <vector>
#include "types.h"

class Payload {
public:
  typedef uint8_t Header;
  typedef uint16_t MaxSize;
  typedef uint16_t Size;
  // ********************
  Payload(MaxSize ms);
  ~Payload();
  // ********************
  const Buffer& buffer() const;
  const void* rawBuffer() const;
  Header header() const;
  Size size() const;
  // ********************
  void* pointWritableBuffer();
protected:
  Buffer myBuff;
  Size mySize;
};

#endif
