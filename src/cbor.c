#include "cbor.h"
#include "cbor_internal.h"

#define assert_avail_bytes(num) do {  \
  if (source_size < (num))            \
    goto missing_source;              \
} while (0)

cbor_item_t * cbor_load(const unsigned char * source, size_t source_size, size_t flags, struct cbor_load_result * result) {
  cbor_item_t * res = malloc(sizeof(cbor_item_t));
  res->refcount = 1;
  result->error = (struct cbor_error) { 0, 0 };
  switch (*source) {
    /* General loops in macros are impossible */
    /* Inline unsigned integer */
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
      {
        result->read = 1;
        res->type = CBOR_TYPE_UINT;
        res->data = malloc(METADATA_WIDTH + 1);
        *(uint8_t *)&res->data[METADATA_WIDTH] = (uint8_t)*source;
        break;
      }
    /* One byte uint */
    case 0x18:
      {
        assert_avail_bytes(2);
        result->read = 2;
        res->type = CBOR_TYPE_UINT;
        res->data = malloc(METADATA_WIDTH + 1);
        *(uint8_t *)&res->data[METADATA_WIDTH] = (uint8_t)*(source + 1);
        break;
      }
  }

  return res;

missing_source:
  result->error = (struct cbor_error) { 0, 1 };
  return NULL;
}

inline cbor_type cbor_typeof(cbor_item_t * item) {
  return item->type;
}

uint_fast8_t cbor_get_uint8(cbor_item_t * item) {
  return *(uint8_t *)(item->data + METADATA_WIDTH);
}
