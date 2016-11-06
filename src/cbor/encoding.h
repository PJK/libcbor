/*
 * Copyright (c) 2014-2016 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_ENCODING_H
#define LIBCBOR_ENCODING_H

#include "cbor/common.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
* ============================================================================
* Primitives encoding
* ============================================================================
*/

/** Primitive encoder */
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

#ifdef __cplusplus
}
#endif


#endif //LIBCBOR_ENCODING_H
