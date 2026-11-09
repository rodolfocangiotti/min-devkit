#include <cassert>
#include <iostream>
#include "TCPSegment.h"
#include "commons.h"

TCPSegment::TCPSegment(const MaxSize ms):
  StreamPayload(ms) {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Constructing TCPSegment class..." << '\n';
#endif
}

TCPSegment::~TCPSegment() {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Destructing TCPSegment class..." << '\n';
#endif
}

void TCPSegment::buildEntryRequest() {
  mySize = sizeof (Header);
  assert(mySize <= myBuff.size());
  Byte* bp = &(myBuff[0]);
  Header* hp = reinterpret_cast<Header*>(bp);
  *hp = ENTRY_REQUEST;
}

void TCPSegment::buildEntryResponse(const ClientToken t) {
  mySize = sizeof (Header) + sizeof (ClientToken);
  assert(mySize <= myBuff.size());
  Byte* bp = &(myBuff[0]);
  Header* hp = reinterpret_cast<Header*>(bp);
  *hp = OKAY_RESPONSE;
  bp = &(myBuff[1]);
  ClientToken* tp = reinterpret_cast<ClientToken*>(bp);
  *tp = t;
}

void TCPSegment::buildExitRequest(const ClientToken t) {
  mySize = sizeof (Header) + sizeof (ClientToken);
  assert(mySize <= myBuff.size());
  Byte* bp = &(myBuff[0]);
  Header* hp = reinterpret_cast<Header*>(bp);
  *hp = EXIT_REQUEST;
  bp = &(myBuff[1]);
  ClientToken* tp = reinterpret_cast<ClientToken*>(bp);
  *tp = t;
}

void TCPSegment::buildExitResponse() {
  mySize = sizeof (Header);
  assert(mySize <= myBuff.size());
  Byte* bp = &(myBuff[0]);
  Header* hp = reinterpret_cast<Header*>(bp);
  *hp = OKAY_RESPONSE;
}
