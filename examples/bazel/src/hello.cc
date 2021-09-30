#include "src/hello.h"

#include "cbor.h"

uint8_t cbor_version() {
  return cbor_major_version;
}

