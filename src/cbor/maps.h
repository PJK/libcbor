/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_MAPS_H
#define LIBCBOR_MAPS_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
* ============================================================================
* Map manipulation
* ============================================================================
*/

size_t cbor_map_size(const cbor_item_t *item);

cbor_item_t *cbor_new_definite_map(const size_t size);

cbor_item_t *cbor_new_indefinite_map();

cbor_item_t *cbor_map_add(cbor_item_t *item, struct cbor_pair pair);

cbor_item_t *cbor_map_add_key(cbor_item_t *item, cbor_item_t *key);

cbor_item_t *cbor_map_add_value(cbor_item_t *item, cbor_item_t *value);

bool cbor_map_is_definite(const cbor_item_t *item);

bool cbor_map_is_indefinite(const cbor_item_t *item);

struct cbor_pair *cbor_map_handle(const cbor_item_t *item);

#ifdef __cplusplus
}
#endif

#endif //LIBCBOR_MAPS_H
