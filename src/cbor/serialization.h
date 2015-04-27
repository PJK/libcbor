/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_SERIALIZATION_H
#define LIBCBOR_SERIALIZATION_H

#include "common.h"

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

#endif //LIBCBOR_SERIALIZATION_H
