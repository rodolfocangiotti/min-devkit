#include <cassert>
#include <iostream>
#include "UDPDatagram.h"
#include "commons.h"

UDPDatagram::UDPDatagram(const MaxSize ms):
  StreamPayload(ms) {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Constructing UDPDatagram class..." << '\n';
#endif
}

UDPDatagram::~UDPDatagram() {
#if defined(DEBUG) && VERBOSENESS > 2
  std::cout << "[DEBUG] Destructing UDPDatagram class..." << '\n';
#endif
}

void UDPDatagram::setParentTID(ClientTID tid) {
  myParentTID = tid;
}

ClientTID UDPDatagram::parentTID() const {
  return myParentTID;
}
