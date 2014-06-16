#include "cbor.h"
#include "cbor_internal.h"

#define assert_avail_bytes(num) do {  \
  if (source_size < (num))            \
    goto missing_source;              \
} while (0)


cbor_item_t * cbor_load(const unsigned char * source, size_t source_size, size_t flags, struct cbor_load_result * result) {
  assert_avail_bytes(1);
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
    /*
     * Do these blocks look redundant to you? They indeed are, for the most part. However, only a demonic macro voodoo
     * could solve this -- and because of the nested expansion semantics, it would have to be about 10 levels deep...
     */
     /* Embedded one byte uint */
      {
        result->read = 1;
        res->type = CBOR_TYPE_UINT;
        res->data = malloc(METADATA_WIDTH + UINT_METADATA_WIDTH + 1);
        *(cbor_uint_width *)&res->data[METADATA_WIDTH] = CBOR_UINT_8;
        *(uint8_t *)&res->data[METADATA_WIDTH + UINT_METADATA_WIDTH] = (uint8_t)*source;
        break;
      }
    /* One byte uint */
    case 0x18:
      {
        assert_avail_bytes(2);
        result->read = 2;
        res->data = malloc(METADATA_WIDTH + UINT_METADATA_WIDTH + 1);
        *(cbor_uint_width *)&res->data[METADATA_WIDTH] = CBOR_UINT_8;
        *(uint8_t *)&res->data[METADATA_WIDTH + UINT_METADATA_WIDTH] = (uint8_t)*(source + 1);
        break;
      }
    /* Two byte uint */
    case 0x19:
      {
        assert_avail_bytes(3);
        result->read = 3;
        res->data = malloc(METADATA_WIDTH + UINT_METADATA_WIDTH + 2);
        *(cbor_uint_width *)&res->data[METADATA_WIDTH] = CBOR_UINT_16;
        /* TODO endianness */
        uint16_t value = ((uint8_t)*(source + 1) << 8) + (uint8_t)*(source + 2);
        *(uint16_t *)&res->data[METADATA_WIDTH + UINT_METADATA_WIDTH] = value;
        break;
      }
    /* Four byte uint */
    case 0x1a:
      {
        assert_avail_bytes(5);
        result->read = 5;
        res->data = malloc(METADATA_WIDTH + UINT_METADATA_WIDTH + 4);
        *(cbor_uint_width *)&res->data[METADATA_WIDTH] = CBOR_UINT_32;
        /* TODO endianness */
        uint32_t value =  ((uint32_t)(uint8_t)*(source + 1) << 0x18) +
                          ((uint32_t)(uint8_t)*(source + 2) << 0x10) +
                          ((uint16_t)(uint8_t)*(source + 3) << 0x08) +
                           (uint8_t )(uint8_t)*(source + 4);
        *(uint32_t *)&res->data[METADATA_WIDTH + UINT_METADATA_WIDTH] = value;
        break;
      }
    /* Glorious eight byte uint */
    case 0x1b:
      {
        assert_avail_bytes(9);
        result->read = 9;
        res->data = malloc(METADATA_WIDTH + UINT_METADATA_WIDTH + 8);
        *(cbor_uint_width *)&res->data[METADATA_WIDTH] = CBOR_UINT_64;
        /* TODO endianness */
        uint64_t value =  ((uint64_t)(uint8_t)*(source + 1) << 0x38) +
                          ((uint64_t)(uint8_t)*(source + 2) << 0x30) +
                          ((uint64_t)(uint8_t)*(source + 3) << 0x28) +
                          ((uint64_t)(uint8_t)*(source + 4) << 0x20) +
                          ((uint32_t)(uint8_t)*(source + 5) << 0x18) +
                          ((uint32_t)(uint8_t)*(source + 6) << 0x10) +
                          ((uint16_t)(uint8_t)*(source + 7) << 0x08) +
                           (uint8_t )(uint8_t)*(source + 8);
        *(uint64_t *)&res->data[METADATA_WIDTH + UINT_METADATA_WIDTH] = value;
        break;
      }
    /* TODO */
    /* Short arrays */
    case 0x80:
      {

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


cbor_uint_width cbor_uint_get_width(cbor_item_t * item) {
  return *(cbor_uint_width *)(item->data + METADATA_WIDTH);
}

uint8_t cbor_get_uint8(cbor_item_t * item) {
  return *(uint8_t *)(item->data + METADATA_WIDTH + UINT_METADATA_WIDTH);
}

uint16_t cbor_get_uint16(cbor_item_t * item) {
  return *(uint16_t *)(item->data + METADATA_WIDTH + UINT_METADATA_WIDTH);
}

uint32_t cbor_get_uint32(cbor_item_t * item) {
  return *(uint32_t *)(item->data + METADATA_WIDTH + UINT_METADATA_WIDTH);
}

uint64_t cbor_get_uint64(cbor_item_t * item) {
  return *(uint64_t *)(item->data + METADATA_WIDTH + UINT_METADATA_WIDTH);
}
/** ========================================================== */

inline bool cbor_isa_uint(cbor_item_t * item) {
	return item->type == CBOR_TYPE_UINT;
}

inline bool cbor_isa_negint(cbor_item_t * item) {
	return item->type == CBOR_TYPE_NEGINT;
}

inline bool cbor_isa_bytestring(cbor_item_t * item) {
	return item->type == CBOR_TYPE_BYTESTRING;
}

inline bool cbor_isa_string(cbor_item_t * item) {
	return item->type == CBOT_TYPE_STRING;
}

inline bool cbor_isa_array(cbor_item_t * item) {
	return item->type == CBOR_TYPE_ARRAY;
}

inline bool cbor_isa_map(cbor_item_t * item) {
	return item->type == CBOR_TYPE_MAP;
}

inline bool cbor_isa_tag(cbor_item_t * item) {
	return item->type == CBOR_TYPE_TAG;
}

inline bool cbor_isa_float(cbor_item_t * item) {
	return item->type == CBOR_TYPE_FLOAT;
}


/** ========================================================== */


inline bool cbor_is_int(cbor_item_t * item) {
  return cbor_isa_uint(item) || cbor_isa_negint(item);
}

inline bool cbor_is_uint(cbor_item_t * item) {
  /* Negative all 'signed' ints are negints */
  return cbor_is_uint(item);
}

inline bool cbor_is_bytestring(cbor_item_t * item) {
}

inline bool cbor_is_string(cbor_item_t * item) {
}

inline bool cbor_is_array(cbor_item_t * item) {
}

inline bool cbor_is_map(cbor_item_t * item) {
}

inline bool cbor_is_float(cbor_item_t * item) {
}

inline bool cbor_is_bool(cbor_item_t * item) {
}

inline bool cbor_is_null(cbor_item_t * item) {
}

inline bool cbor_is_undef(cbor_item_t * item) {
}
