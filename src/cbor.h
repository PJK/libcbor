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
_Static_assert(sizeof(bool) == 1, "size_t must be at least 64 bits");


/*
 * Packed bitfield
 */
// TODO ensure this works on big endian systems (I doubt it)
typedef union {
	struct {
		bool no_realloc : 1;
		bool canonical  : 1;
		int : sizeof(int) * 8 - 3; /* Padding */
	};
	int raw;
} cbor_flags_t;

#define CBOR_FLAGS_NONE ((cbor_flags_t) { { 0 } })

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
	CBOR_CTRL_BREAK,
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
	_CBOR_METADATA_MADE		 = 0x04 	/* Created by a cbor_make_<X> */
} _cbor_metadata;

struct _cbor_int_metadata {
	cbor_int_width width;
};

typedef enum {
	_CBOR_STRING_METADATA_DEFINITE,
	_CBOR_STRING_METADATA_INDEFINITE
} _cbor_string_type_metadata;

struct _cbor_bytestring_metadata {
	size_t length;
	_cbor_string_type_metadata type;
};

struct _cbor_string_metadata {
	size_t length;
	_cbor_string_type_metadata type;
};

typedef enum {
	_CBOR_ARRAY_METADATA_DEFINITE,
	_CBOR_ARRAY_METADATA_INDEFINITE
} _cbor_array_type_metadata;

struct _cbor_array_metadata {
	size_t size;
	_cbor_array_type_metadata type;
};

struct _cbor_map_metadata {
	size_t size;
};

struct _cbor_float_ctrl_metadata {
	cbor_float_width width;
	cbor_ctrl type;
};

union cbor_item_metadata {
	struct _cbor_int_metadata		 int_metadata;
	struct _cbor_bytestring_metadata bytestring_metadata;
	struct _cbor_string_metadata	 string_metadata;
	struct _cbor_array_metadata		 array_metadata;
	struct _cbor_map_metadata		 map_metadata;
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

struct cbor_pair {
	cbor_item_t * key, value;
};

typedef struct cbor_map_iterator {
	struct cbor_pair data;
	// METADATA FOR A LINKED LIST OR TREE OR WHATEVER
} cbor_map_iterator;

struct cbor_load_result {
	struct cbor_error error;
	size_t			  read;
};

enum cbor_callback_result {
	CBOR_CALLBACK_OK,
	CBOR_CALLBACK_SKIP //TODO
};

typedef const unsigned char * cbor_data;

typedef enum cbor_callback_result(* cbor_int8_callback)(uint8_t);
typedef enum cbor_callback_result(* cbor_int16_callback)(uint16_t);
typedef enum cbor_callback_result(* cbor_int32_callback)(uint32_t);
typedef enum cbor_callback_result(* cbor_int64_callback)(uint64_t);
typedef enum cbor_callback_result(* cbor_simple_callback)();
typedef enum cbor_callback_result(* cbor_string_callback)(cbor_data, size_t);
typedef enum cbor_callback_result(* cbor_collection_callback)(size_t);
typedef enum cbor_callback_result(* cbor_float_callback)(float);
typedef enum cbor_callback_result(* cbor_double_callback)(double);
typedef enum cbor_callback_result(* cbor_bool_callback)(bool);

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

	/* Type 7 - Floats & misc */
	/* Type names cannot be member names */
	cbor_double_callback float2; /* 2B float is not supported in standard C */
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

struct cbor_decoder_result cbor_stream_decode(cbor_data, size_t, const struct cbor_callbacks *);

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

/* int manipulation - both uint and negint*/
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

/* Bytestrings manipulation*/
size_t cbor_bytestring_length(const cbor_item_t * item);

bool cbor_bytestring_is_definite(const cbor_item_t * item);
bool cbor_bytestring_is_indefinite(const cbor_item_t * item);

unsigned char * cbor_bytestring_handle(const cbor_item_t * item);

/* has to be called at least one - to decref the chunk */
cbor_item_t * cbor_bytestring_get_chunk(const cbor_item_t * item);

/* once you call this, previous chunk is lost (avoiding realloc)*/
void cbor_bytestring_read_chunk(cbor_item_t * item, const unsigned char * source, size_t source_size, struct cbor_load_result * result);

cbor_item_t * cbor_new_definite_bytestring();
cbor_item_t * cbor_new_indefinite_bytestring();

void cbor_bytestring_set_handle(cbor_item_t * item, unsigned char * data, size_t length);
void cbor_bytestring_set_chunk(cbor_item_t * item, const cbor_item_t * chunk);

// TODO rename this / figure out gets/sets verbs
size_t cbor_array_get_size(cbor_item_t * item);
bool cbor_array_is_definite(cbor_item_t * item);
bool cbor_array_is_indefinite(cbor_item_t * item);
/* Native handle to the underlying chunk */
cbor_item_t ** cbor_array_handle(cbor_item_t * item);

size_t cbor_map_size(cbor_item_t * item);
cbor_item_t * cbor_new_map();
struct cbor_map_iterator cbor_map_add(cbor_item_t * item, struct cbor_pair pair);
void cbor_map_is_definite(cbor_item_t * item);
void cbor_map_is_indefinite(cbor_item_t * item);
struct cbor_map_iterator cbor_map_begin(cbor_item_t * item);
bool cbor_map_iterator_end(struct cbor_map_iterator * iter);
void cbor_map_iterator_next(struct cbor_map_iterator * iter);
void cbor_map_delete(struct cbor_map_iterator * iter);


cbor_float_width cbor_float_ctrl_get_width(const cbor_item_t * item);
cbor_ctrl cbor_float_ctrl_get_ctrl(const cbor_item_t * item);

#ifdef DEBUG
#include <stdio.h>
/* Pretty-print item description to STDOUT */
void cbor_describe(cbor_item_t * item);
#endif

#endif
