#ifndef CBOR_H_
#define CBOR_H_


#include <stdio.h>

#define CBOR_MAJOR_VERSION 0
#define CBOR_MINOR_VERSION 0
#define CBOR_PATCH_VERSION 1

#include "cbor/common.h"

#define CBOR_VERSION TO_STR(CBOR_MAJOR_VERSION) "." TO_STR(CBOR_MINOR_VERSION) "." TO_STR(CBOR_PATCH_VERSION)

_Static_assert(sizeof(size_t) >= 8, "size_t must be at least 64 bits"); /* Otherwise we cannot support reasonably sized chunks */

#include "cbor/arrays.h"
#include "cbor/bytestrings.h"
#include "cbor/data.h"
#include "cbor/floats_ctrls.h"
#include "cbor/ints.h"
#include "cbor/maps.h"
#include "cbor/strings.h"
#include "cbor/tags.h"

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


/*
* ============================================================================
* High level decoding
* ============================================================================
*/

struct cbor_decoder_result cbor_stream_decode(cbor_data, size_t, const struct cbor_callbacks *, void *);
cbor_item_t * cbor_load(cbor_data source, size_t source_size, struct cbor_load_result * result);


/*
* ============================================================================
* Primitives encoding
* ============================================================================
*/
/*!
 * Copies bytes from a source memory area to a destination memory area,
 * where both areas may not overlap.
 * @param[in] dest The memory area to copy to.
 * @param[in]  src  The memory area to copy from.
 * @param[in]  n    The number of bytes to copy
 */
size_t cbor_encode_uint8(uint8_t, unsigned char *, size_t);
size_t cbor_encode_uint16(uint16_t, unsigned char *, size_t);
size_t cbor_encode_uint32(uint32_t, unsigned char *, size_t);
size_t cbor_encode_uint64(uint64_t, unsigned char *, size_t);
size_t cbor_encode_uint(uint64_t, unsigned char *, size_t);

size_t cbor_encode_negint8(uint8_t, unsigned char *, size_t);
size_t cbor_encode_negint16(uint16_t, unsigned char *, size_t);
size_t cbor_encode_negint32(uint32_t, unsigned char *, size_t);
size_t cbor_encode_negint64(uint64_t, unsigned char *, size_t);
size_t cbor_encode_negint(uint64_t, unsigned char *, size_t);

size_t cbor_encode_bytestring_start(size_t, unsigned char *, size_t);
size_t cbor_encode_indef_bytestring_start(unsigned char *, size_t);

size_t cbor_encode_string_start(size_t, unsigned char *, size_t);
size_t cbor_encode_indef_string_start(unsigned char *, size_t);

size_t cbor_encode_array_start(size_t, unsigned char *, size_t);
size_t cbor_encode_indef_array_start(unsigned char *, size_t);

size_t cbor_encode_map_start(size_t, unsigned char *, size_t);
size_t cbor_encode_indef_map_start(unsigned char *, size_t);

size_t cbor_encode_tag(uint64_t, unsigned char *, size_t);

size_t cbor_encode_bool(bool, unsigned char *, size_t);
size_t cbor_encode_null(unsigned char *, size_t);
size_t cbor_encode_undef(unsigned char *, size_t);
size_t cbor_encode_half(float, unsigned char *, size_t);
size_t cbor_encode_single(float, unsigned char *, size_t);
size_t cbor_encode_double(double, unsigned char *, size_t);
size_t cbor_encode_break(unsigned char *, size_t);
size_t cbor_encode_ctrl(uint8_t, unsigned char *, size_t);


/*
* ============================================================================
* High level encoding
* ============================================================================
*/

size_t cbor_serialize(const cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_uint(const cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_negint(const cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_bytestring(const cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_string(const cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_array(const cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_map(const cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_tag(const cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_float_ctrl(const cbor_item_t *, unsigned char *, size_t);




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

/* Build initialized integers - new + set_uint + mark_uint */
cbor_item_t * cbor_build_uint8(uint8_t value);
cbor_item_t * cbor_build_uint16(uint16_t value);
cbor_item_t * cbor_build_uint32(uint32_t value);
cbor_item_t * cbor_build_uint64(uint64_t value);





/*
* ============================================================================
* Map manipulation
* ============================================================================
*/

size_t cbor_map_size(const cbor_item_t * item);
cbor_item_t * cbor_new_definite_map(const size_t size);
cbor_item_t * cbor_new_indefinite_map();
cbor_item_t * cbor_map_add(cbor_item_t * item, struct cbor_pair pair);
cbor_item_t * cbor_map_add_key(cbor_item_t * item, cbor_item_t * key);

bool cbor_map_is_definite(const cbor_item_t * item);
bool cbor_map_is_indefinite(const cbor_item_t * item);
struct cbor_pair * cbor_map_handle(const cbor_item_t * item);

/*
* ============================================================================
* Tag manipulation
* ============================================================================
*/

cbor_item_t * cbor_new_tag(uint64_t value);
cbor_item_t * cbor_tag_item(const cbor_item_t * item);
uint64_t cbor_tag_value(const cbor_item_t * item);
void cbor_tag_set_item(cbor_item_t * item, cbor_item_t * tagged_item);



#ifdef DEBUG
#include <stdio.h>
/* Pretty-print item description to STDOUT */
void cbor_describe(cbor_item_t * item);
#endif

#endif
