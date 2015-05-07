/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_ARRAYS_H
#define LIBCBOR_ARRAYS_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t cbor_array_size(const cbor_item_t * item);
cbor_item_t * cbor_array_get(const cbor_item_t * item, size_t index);
void cbor_array_set(cbor_item_t * item, size_t index, cbor_item_t * value);
void cbor_array_replace(cbor_item_t * item, size_t index, cbor_item_t * value);

bool cbor_array_is_definite(const cbor_item_t * item);
bool cbor_array_is_indefinite(const cbor_item_t * item);
/* Native handle to the underlying chunk */
cbor_item_t ** cbor_array_handle(const cbor_item_t * item);

cbor_item_t * cbor_new_definite_array(const size_t);
cbor_item_t * cbor_new_indefinite_array();

cbor_item_t * cbor_array_push(cbor_item_t * array, cbor_item_t * pushee);

#ifdef __cplusplus
}
#endif

#endif //LIBCBOR_ARRAYS_H
