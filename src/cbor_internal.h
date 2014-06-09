#ifndef CBOR_INTERNAL_
#define CBOR_INTERNAL_

#include "cbor.h"

typedef enum {
  METADATA_EMPTY    = 0x00,
  METADATA_COMPLETE = 0x01
} metadata;

#define METADATA_WIDTH sizeof(metadata)

#define UINT_METADATA_WIDTH sizeof(cbor_uint_width)

#endif
