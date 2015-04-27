/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_BYTESTRINGS_H
#define LIBCBOR_BYTESTRINGS_H

#include "common.h"

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


#endif //LIBCBOR_BYTESTRINGS_H
