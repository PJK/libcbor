#ifndef CBOR_H_
#define CBOR_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "magic.h"

#define CBOR_MAJOR_VERSION 0
#define CBOR_MINOR_VERSION 0
#define CBOR_PATCH_VERSION 1

#define CBOR_VERSION TO_STR(CBOR_MAJOR_VERSION) "." TO_STR(CBOR_MINOR_VERSION) "." TO_STR(CBOR_PATCH_VERSION)

/*
 * TODO - work around this on <=32 bit platforms - now arrays and bytestrings can
 * be up to 2^64-1 items/bytes long -- how do we ensure real size won't overflow
 * size_t???
 */
_Static_assert(sizeof(size_t) >= 8, "size_t must be at least 64-bits");

typedef enum {
	CBOR_TYPE_UINT,        /* 0 */
	CBOR_TYPE_NEGINT,      /* 1 */
	CBOR_TYPE_BYTESTRING,  /* 2 */
	CBOT_TYPE_STRING,      /* 3 */
	CBOR_TYPE_ARRAY,       /* 4 */
	CBOR_TYPE_MAP,         /* 5 */
	CBOR_TYPE_TAG,         /* 6 - additional semantics*/
	CBOR_TYPE_FLOAT_CTRL   /* 7 - also bool, null, undefined, and others */
} cbor_type;

typedef enum {
	  CBOR_ERR_NONE,
	  CBOR_ERR_NOTENOUGHDATA,
	  CBOR_ERR_NODATA
} cbor_error_code;

/* Possible widths of CBOR_TYPE_UINT */
typedef enum {
	CBOR_INT_8,
	CBOR_INT_16,
	CBOR_INT_32,
	CBOR_INT_64
} cbor_int_width;

typedef enum {
	CBOR_FLOAT_0, /* Registered for internal use - breaks and such*/
	CBOR_FLOAT_16,
	CBOR_FLOAT_32,
	CBOR_FLOAT_64
} cbor_float_width;

typedef enum {
	CBOR_CTRL_NONE,
	CBOR_CTRL_BREAK
} cbor_ctrl;

// TODO hook these into the parser
typedef enum {
	_CBOR_METADATA_EMPTY	 = 0x00,
	_CBOR_METADATA_COMPLETE  = 0x01,
	_CBOR_METADATA_RESUMABLE = 0x02,	/* Parsing may be resumed */
	_CBOR_METADATA_MADE		 = 0x04 	/* Created by a cbor_make_<X> */
} _cbor_metadata;

struct _cbor_int_metadata {
	cbor_int_width width;
};

typedef enum {
	_CBOR_BYTESTRING_METADATA_DEFINITE,
	_CBOR_BYTESTRING_METADATA_INDEFINITE
} _cbor_bytestring_type_metadata;

struct _cbor_bytestring_metadata {
	size_t length;
	_cbor_bytestring_type_metadata type;
};

typedef enum {
	_CBOR_ARRAY_METADATA_DEFINITE,
	_CBOR_ARRAY_METADATA_INDEFINITE
} _cbor_array_type_metadata;

struct _cbor_array_metadata {
	size_t size;
	_cbor_array_type_metadata type;
};

struct _cbor_float_ctrl_metadata {
	cbor_float_width width;
	cbor_ctrl type;
};

union cbor_item_metadata {
	struct _cbor_int_metadata		 int_metadata;
	struct _cbor_bytestring_metadata bytestring_metadata;
	struct _cbor_array_metadata		 array_metadata;
	struct _cbor_float_ctrl_metadata float_ctrl_metadata;
};

typedef struct cbor_item_t {
	cbor_type				 type;
	size_t					 refcount;
	union cbor_item_metadata metadata;
	unsigned char * 		 data;
} cbor_item_t;

struct cbor_error {
	size_t			position;
	cbor_error_code code;
};

struct cbor_load_result {
	struct cbor_error error;
	size_t			  read;
};

cbor_item_t * cbor_load(const unsigned char * source, size_t source_size, size_t flags, struct cbor_load_result * result);

void cbor_incref(cbor_item_t * item);
void cbor_decref(cbor_item_t ** item);

cbor_type cbor_typeof(cbor_item_t * item); /* will be inlined iff link-time opt is enabled */

/* Standard item types as described by the RFC */
bool cbor_isa_uint(cbor_item_t * item);
bool cbor_isa_negint(cbor_item_t * item);
bool cbor_isa_bytestring(cbor_item_t * item);
bool cbor_isa_string(cbor_item_t * item);
bool cbor_isa_array(cbor_item_t * item);
bool cbor_isa_map(cbor_item_t * item);
bool cbor_isa_tag(cbor_item_t * item);
bool cbor_isa_float_ctrl(cbor_item_t * item);

/* Practical types with respect to their semantics (but no tag values) */
bool cbor_is_int(cbor_item_t * item);
bool cbor_is_uint(cbor_item_t * item);
bool cbor_is_float(cbor_item_t * item);
bool cbor_is_bool(cbor_item_t * item);
bool cbor_is_null(cbor_item_t * item);
bool cbor_is_undef(cbor_item_t * item);

/* uint manipulation - both uint and negint*/
uint8_t cbor_get_uint8(cbor_item_t * item);
uint16_t cbor_get_uint16(cbor_item_t * item);
uint32_t cbor_get_uint32(cbor_item_t * item);
uint64_t cbor_get_uint64(cbor_item_t * item);

void cbor_set_uint8(cbor_item_t * item, uint8_t value);
void cbor_set_uint16(cbor_item_t * item, uint16_t value);
void cbor_set_uint32(cbor_item_t * item, uint32_t value);
void cbor_set_uint64(cbor_item_t * item, uint64_t value);

cbor_int_width cbor_int_get_width(cbor_item_t * item);

void cbor_mark_uint(cbor_item_t * item);
void cbor_mark_negint(cbor_item_t * item);
cbor_item_t * cbor_new_int8();
cbor_item_t * cbor_new_int16();
cbor_item_t * cbor_new_int32();
cbor_item_t * cbor_new_int64();

size_t cbor_bytestring_length(cbor_item_t * item);
unsigned char * cbor_bytestring_handle(cbor_item_t * item);
bool cbor_bytestring_is_definite(cbor_item_t * item);
bool cbor_bytestring_is_indefinite(cbor_item_t * item);
/* has to be called at least one - to decref the chunk */
cbor_item_t * cbor_bytestring_get_chunk(cbor_item_t * item);
/* once you call this, previous chunk is lost (avoiding realloc)*/
void cbor_bytestring_read_chunk(cbor_item_t * item, const unsigned char * source, size_t source_size, struct cbor_load_result * result);

size_t cbor_array_get_size(cbor_item_t * item);
bool cbor_array_is_definite(cbor_item_t * item);
bool cbor_array_is_indefinite(cbor_item_t * item);
/* Native handle to the underlying chunk */
cbor_item_t ** cbor_array_handle(cbor_item_t * item);

cbor_float_width cbor_float_ctrl_get_width(cbor_item_t * item);
cbor_ctrl cbor_float_ctrl_get_ctrl(cbor_item_t * item);

#ifdef DEBUG
#include <stdio.h>
/* Pretty-print item description to STDOUT */
void cbor_describe(cbor_item_t * item);
#endif

#endif
