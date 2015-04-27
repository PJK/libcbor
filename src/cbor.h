#ifndef CBOR_H_
#define CBOR_H_


#include <stdio.h>

#include "cbor/common.h"


#include "cbor/arrays.h"
#include "cbor/bytestrings.h"
#include "cbor/data.h"
#include "cbor/floats_ctrls.h"
#include "cbor/ints.h"
#include "cbor/maps.h"
#include "cbor/strings.h"
#include "cbor/tags.h"

#include "cbor/callbacks.h"







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












#ifdef DEBUG
#include <stdio.h>
/* Pretty-print item description to STDOUT */
void cbor_describe(cbor_item_t * item);
#endif

#endif
