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
 * TODO work around this on <=32 bit platforms - now arrays and bytestrings can
 * be up to 2^64-1 items/bytes long -- how do we ensure real size won't overflow
 * size_t???
 */
_Static_assert(sizeof(size_t) >= 8, "size_t must be at least 64 bits");

/*
 * Packed bitfield
 */
// TODO ensure this works on big endian systems (I doubt it)
typedef union {
	struct {
		bool no_realloc : 1;
		bool canonical  : 1;
		int : sizeof(int) * 8 - 2; /* Padding */
	};
	int raw;
} cbor_flags_t;

#define CBOR_FLAGS_NONE ((cbor_flags_t) { { 0 } })

typedef enum {
	CBOR_TYPE_UINT,        /* 0 */
	CBOR_TYPE_NEGINT,      /* 1 */
	CBOR_TYPE_BYTESTRING,  /* 2 */
	CBOR_TYPE_STRING,      /* 3 */
	CBOR_TYPE_ARRAY,       /* 4 */
	CBOR_TYPE_MAP,         /* 5 */
	CBOR_TYPE_TAG,         /* 6 - additional semantics*/
	CBOR_TYPE_FLOAT_CTRL   /* 7 - also bool, null, undefined, and others */
} cbor_type;

typedef enum {
	CBOR_ERR_NONE,
	CBOR_ERR_NOTENOUGHDATA,
	CBOR_ERR_NODATA,
	CBOR_ERR_MALFORMATED
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
	CBOR_CTRL_FALSE,
	CBOR_CTRL_TRUE,
	CBOR_CTRL_NULL,
	CBOR_CTRL_UNDEF
} cbor_ctrl;

// TODO hook these into the parser
typedef enum {
	_CBOR_METADATA_EMPTY	 = 0x00,
	_CBOR_METADATA_COMPLETE  = 0x01,
	_CBOR_METADATA_RESUMABLE = 0x02,	/* Parsing may be resumed */
} _cbor_metadata;

typedef enum {
	_CBOR_METADATA_DEFINITE,
	_CBOR_METADATA_INDEFINITE
} _cbor_dst_metadata;


struct _cbor_int_metadata {
	cbor_int_width width;
};

struct _cbor_bytestring_metadata {
	size_t length;
	_cbor_dst_metadata type;
};

struct _cbor_string_metadata {
	size_t             length;
	size_t             codepoint_count; /* Sum of chunks' codepoint_counts for indefinite strings */
	_cbor_dst_metadata type;
};

struct _cbor_array_metadata {
	size_t             size;
	_cbor_dst_metadata type;
};

struct _cbor_map_metadata {
	size_t size;
	_cbor_dst_metadata type;
};

/* cbor_item_metadata is 2 * sizeof(size_t) + sizeof(_cbor_string_type_metadata), lets use the space */
struct _cbor_tag_metadata {
	struct cbor_item_t * tagged_item;
	uint64_t             value;
};

struct _cbor_float_ctrl_metadata {
	cbor_float_width width;
	cbor_ctrl        type;
};

union cbor_item_metadata {
	struct _cbor_int_metadata		 int_metadata;
	struct _cbor_bytestring_metadata bytestring_metadata;
	struct _cbor_string_metadata	 string_metadata;
	struct _cbor_array_metadata		 array_metadata;
	struct _cbor_map_metadata		 map_metadata;
	struct _cbor_tag_metadata		 tag_metadata;
	struct _cbor_float_ctrl_metadata float_ctrl_metadata;
};

typedef struct cbor_item_t {
	cbor_type				 type;
	size_t					 refcount;
	union cbor_item_metadata metadata;
	unsigned char * 		 data;
} cbor_item_t;

struct cbor_indefinite_string_data {
	size_t          chunk_count;
	size_t          chunk_capacity;
	cbor_item_t * * chunks;
};

struct cbor_error {
	size_t			position;
	cbor_error_code code;
};

struct cbor_pair {
	cbor_item_t * key, * value;
};

struct cbor_load_result {
	struct cbor_error error;
	size_t            read;
};

enum cbor_callback_result {
	CBOR_CALLBACK_OK,
	CBOR_CALLBACK_SKIP //TODO
};

typedef const unsigned char * cbor_data;

typedef enum cbor_callback_result(* cbor_int8_callback)(void *, uint8_t);
typedef enum cbor_callback_result(* cbor_int16_callback)(void *, uint16_t);
typedef enum cbor_callback_result(* cbor_int32_callback)(void *, uint32_t);
typedef enum cbor_callback_result(* cbor_int64_callback)(void *, uint64_t);
typedef enum cbor_callback_result(* cbor_simple_callback)(void *);
typedef enum cbor_callback_result(* cbor_string_callback)(void *, cbor_data, size_t);
typedef enum cbor_callback_result(* cbor_collection_callback)(void *, size_t);
typedef enum cbor_callback_result(* cbor_float_callback)(void *, float);
typedef enum cbor_callback_result(* cbor_double_callback)(void *, double);
typedef enum cbor_callback_result(* cbor_bool_callback)(void *, bool);

struct cbor_callbacks {
	/* Type 0 - Unsigned integers */
	cbor_int8_callback uint8;
	cbor_int16_callback uint16;
	cbor_int32_callback uint32;
	cbor_int64_callback uint64;

	/* Type 1 - Negative integers */
	cbor_int8_callback negint8;
	cbor_int16_callback negint16;
	cbor_int32_callback negint32;
	cbor_int64_callback negint64;

	/* Type 2 - Byte strings */
	cbor_string_callback byte_string;
	cbor_simple_callback byte_string_start;

	/* Type 3 - Strings */
	cbor_string_callback string;
	cbor_simple_callback string_start;

	/* Type 4 - Arrays */
	cbor_collection_callback array_start;
	cbor_simple_callback indef_array_start;

	/* Type 5 - Maps */
	cbor_collection_callback map_start;
	cbor_simple_callback indef_map_start;

	/* Type 6 - Tags */
	cbor_int64_callback tag;

	/* Type 7 - Floats & misc */
	/* Type names cannot be member names */
	cbor_float_callback float2; /* 2B float is not supported in standard C */
	cbor_float_callback float4;
	cbor_double_callback float8;
	cbor_simple_callback undefined;
	cbor_simple_callback null;
	cbor_bool_callback boolean;

	/* Shared indefinites */
	cbor_simple_callback indef_break;
};

enum cbor_decoder_status {
	CBOR_DECODER_FINISHED,
	CBOR_DECODER_NEDATA,
	CBOR_DECODER_EBUFFER,
	CBOR_DECODER_ERROR
};

struct cbor_decoder_result {
	size_t                   read;
	enum cbor_decoder_status status;
};

struct cbor_decoder_result cbor_stream_decode(cbor_data, size_t, const struct cbor_callbacks *, void *);

size_t cbor_encode_uint8(uint8_t, unsigned char *, size_t);
size_t cbor_encode_uint16(uint16_t, unsigned char *, size_t);
size_t cbor_encode_uint32(uint32_t, unsigned char *, size_t);
size_t cbor_encode_uint64(uint64_t, unsigned char *, size_t);

size_t cbor_encode_uint(uint64_t, unsigned char *, size_t);


cbor_item_t * cbor_load(cbor_data source, size_t source_size, cbor_flags_t flags, struct cbor_load_result * result);

void cbor_incref(cbor_item_t * item);
void cbor_decref(cbor_item_t ** item);

cbor_type cbor_typeof(const cbor_item_t * item); /* Will be inlined iff link-time opt is enabled */

/* Standard item types as described by the RFC */
bool cbor_isa_uint(const cbor_item_t * item);
bool cbor_isa_negint(const cbor_item_t * item);
bool cbor_isa_bytestring(const cbor_item_t * item);
bool cbor_isa_string(const cbor_item_t * item);
bool cbor_isa_array(const cbor_item_t * item);
bool cbor_isa_map(const cbor_item_t * item);
bool cbor_isa_tag(const cbor_item_t * item);
bool cbor_isa_float_ctrl(const cbor_item_t * item);

/* Practical types with respect to their semantics (but no tag values) */
bool cbor_is_int(const cbor_item_t * item);
bool cbor_is_uint(const cbor_item_t * item);
bool cbor_is_float(const cbor_item_t * item);
bool cbor_is_bool(const cbor_item_t * item);
bool cbor_is_null(const cbor_item_t * item);
bool cbor_is_undef(const cbor_item_t * item);

/*
* ============================================================================
* Integer (uints and negints) manipulation
* ============================================================================
*/

uint8_t cbor_get_uint8(const cbor_item_t * item);
uint16_t cbor_get_uint16(const cbor_item_t * item);
uint32_t cbor_get_uint32(const cbor_item_t * item);
uint64_t cbor_get_uint64(const cbor_item_t * item);

void cbor_set_uint8(cbor_item_t * item, uint8_t value);
void cbor_set_uint16(cbor_item_t * item, uint16_t value);
void cbor_set_uint32(cbor_item_t * item, uint32_t value);
void cbor_set_uint64(cbor_item_t * item, uint64_t value);

cbor_int_width cbor_int_get_width(const cbor_item_t * item);

void cbor_mark_uint(cbor_item_t * item);
void cbor_mark_negint(cbor_item_t * item);

cbor_item_t * cbor_new_int8();
cbor_item_t * cbor_new_int16();
cbor_item_t * cbor_new_int32();
cbor_item_t * cbor_new_int64();

/*
* ============================================================================
* String manipulation
* ============================================================================
*/

size_t cbor_string_length(const cbor_item_t * item);
bool cbor_string_is_definite(const cbor_item_t * item);
bool cbor_string_is_indefinite(const cbor_item_t * item);
unsigned char * cbor_string_handle(const cbor_item_t * item);
size_t cbor_string_codepoint_count(const cbor_item_t * item);

void cbor_string_set_handle(cbor_item_t * item, unsigned char * data, size_t length);

/* Indefinite strings only */
cbor_item_t * * cbor_string_chunks_handle(const cbor_item_t * item);
size_t cbor_string_chunk_count(const cbor_item_t * item);

/* Returns NULL on realloc failure */
cbor_item_t * cbor_string_add_chunk(cbor_item_t * item, cbor_item_t * chunk);
cbor_item_t * cbor_string_concatenate(cbor_item_t * item);

cbor_item_t * cbor_new_definite_string();
cbor_item_t * cbor_new_indefinite_string();


/*
* ============================================================================
* Byte string manipulation
* ============================================================================
*/

size_t cbor_bytestring_length(const cbor_item_t * item);
bool cbor_bytestring_is_definite(const cbor_item_t * item);
bool cbor_bytestring_is_indefinite(const cbor_item_t * item);
unsigned char * cbor_bytestring_handle(const cbor_item_t * item);

void cbor_bytestring_set_handle(cbor_item_t * item, unsigned char * data, size_t length);

/* Indefinite bytestrings only */
cbor_item_t * * cbor_bytestring_chunks_handle(const cbor_item_t * item);
size_t cbor_bytestring_chunk_count(const cbor_item_t * item);

/* Returns NULL on realloc failure */
cbor_item_t * cbor_bytestring_add_chunk(cbor_item_t * item, cbor_item_t * chunk);
cbor_item_t * cbor_bytestring_concatenate(cbor_item_t * item);

cbor_item_t * cbor_new_definite_bytestring();
cbor_item_t * cbor_new_indefinite_bytestring();

/*
* ============================================================================
* Array manipulation
* ============================================================================
*/

size_t cbor_array_size(cbor_item_t * item);
bool cbor_array_is_definite(cbor_item_t * item);
bool cbor_array_is_indefinite(cbor_item_t * item);
/* Native handle to the underlying chunk */
cbor_item_t ** cbor_array_handle(cbor_item_t * item);

cbor_item_t * cbor_new_definite_array(size_t);
cbor_item_t * cbor_new_indefinite_array();

cbor_item_t * cbor_array_push(cbor_item_t * array, cbor_item_t * pushee);

/*
* ============================================================================
* Map manipulation
* ============================================================================
*/

size_t cbor_map_size(cbor_item_t * item);
cbor_item_t * cbor_new_definite_map(size_t size);
cbor_item_t * cbor_new_indefinite_map();
cbor_item_t * cbor_map_add(cbor_item_t * item, struct cbor_pair pair);
bool cbor_map_is_definite(cbor_item_t * item);
bool cbor_map_is_indefinite(cbor_item_t * item);
struct cbor_pair * cbor_map_handle(cbor_item_t * item);

/*
* ============================================================================
* Tag manipulation
* ============================================================================
*/

cbor_item_t * cbor_new_tag(uint64_t value);
cbor_item_t * cbor_tag_item(const cbor_item_t * item);
uint64_t cbor_tag_value(const cbor_item_t * item);
void cbor_tag_set_item(cbor_item_t * item, cbor_item_t * tagged_item);

/*
* ============================================================================
* Float manipulation
* ============================================================================
*/

bool cbor_float_ctrl_is_ctrl(const cbor_item_t * item);
cbor_float_width cbor_float_get_width(const cbor_item_t * item);

float cbor_float_get_float2(const cbor_item_t * item);
float cbor_float_get_float4(const cbor_item_t * item);
double cbor_float_get_float8(const cbor_item_t * item);

cbor_item_t * cbor_new_ctrl(); /* float0 */
cbor_item_t * cbor_new_float2();
cbor_item_t * cbor_new_float4();
cbor_item_t * cbor_new_float8();

cbor_item_t * cbor_new_null();
cbor_item_t * cbor_new_undef();
cbor_item_t * cbor_new_bool(bool value);

void cbor_set_ctrl(cbor_item_t * item, cbor_ctrl value);
void cbor_set_float2(cbor_item_t * item, float value);
void cbor_set_float4(cbor_item_t * item, float value);
void cbor_set_float8(cbor_item_t * item, double value);

cbor_ctrl cbor_ctrl_code(const cbor_item_t * item);
bool cbor_ctrl_bool(const cbor_item_t * item);

#ifdef DEBUG
#include <stdio.h>
/* Pretty-print item description to STDOUT */
void cbor_describe(cbor_item_t * item);
#endif

#endif
