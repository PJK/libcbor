/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_INTS_H
#define LIBCBOR_INTS_H

#include "common.h"

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

#endif //LIBCBOR_INTS_H
