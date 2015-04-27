/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_STRINGS_H
#define LIBCBOR_STRINGS_H

#include "common.h"

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


#endif //LIBCBOR_STRINGS_H
