#ifndef CBOR_H_
#define CBOR_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "magic.h"

#define CBOR_MAJOR_VERSION 0
#define CBOR_MINOR_VERSION 0
#define CBOR_PATCH_VERSION 0

#define CBOR_VERSION TO_STR(CBOR_MAJOR_VERSION) "." TO_STR(CBOR_MINOR_VERSION) "." TO_STR(CBOR_PATCH_VERSION)

typedef enum {
  CBOR_TYPE_UINT,        /* 0 */
  CBOR_TYPE_NEGINT,      /* 1 */
  CBOR_TYPE_BYTESTRING,  /* 2 */
  CBOT_TYPE_STRING,      /* 3 */
  CBOR_TYPE_ARRAY,       /* 4 */
  CBOR_TYPE_MAP,         /* 5 */
  CBOR_TYPE_TAG,         /* 6 - additional semantics*/
  CBOR_TYPE_FLOAT        /* 7 - also bool, null, undefined, and others */
} cbor_type;

/* Possible widths of CBOR_TYPE_UINT */
typedef enum {
  CBOR_UINT_8,
  CBOR_UINT_16,
  CBOR_UINT_32,
  CBOR_UINT_64
} cbor_uint_width;

typedef struct cbor_item_t {
  cbor_type type;
  size_t  refcount;
  char *  data;
} cbor_item_t;

struct cbor_error {
  size_t  postion;
  int     code;
};

struct cbor_load_result {
  struct cbor_error error;
  size_t            read;
};

cbor_item_t * cbor_load(const unsigned char * source, size_t source_size, size_t flags, struct cbor_load_result * result);

cbor_item_t * cbor_incref(cbor_item_t * item);
cbor_item_t * cbor_decref(cbor_item_t * item);

cbor_type cbor_typeof(cbor_item_t * item); /* will be inlined iff link-time opt is enabled */

/* Standard item types as described by the RFC */
bool cbor_isa_uint(cbor_item_t * item);
bool cbor_isa_negint(cbor_item_t * item);
bool cbor_isa_bytestring(cbor_item_t * item);
bool cbor_isa_string(cbor_item_t * item);
bool cbor_isa_array(cbor_item_t * item);
bool cbor_isa_map(cbor_item_t * item);
bool cbor_isa_tag(cbor_item_t * item);
bool cbor_isa_float(cbor_item_t * item);

/* Practical types with respect to their semantics (but no tag values) */
bool cbor_is_int(cbor_item_t * item);
bool cbor_is_uint(cbor_item_t * item);
bool cbor_is_bytestring(cbor_item_t * item);
bool cbor_is_string(cbor_item_t * item);
bool cbor_is_array(cbor_item_t * item);
bool cbor_is_map(cbor_item_t * item);
bool cbor_is_float(cbor_item_t * item);
bool cbor_is_bool(cbor_item_t * item);
bool cbor_is_null(cbor_item_t * item);
bool cbor_is_undef(cbor_item_t * item);

/* uint manipulation */
uint8_t cbor_get_uint8(cbor_item_t * item);
uint16_t cbor_get_uint16(cbor_item_t * item);
uint32_t cbor_get_uint32(cbor_item_t * item);
uint64_t cbor_get_uint64(cbor_item_t * item);

#endif
