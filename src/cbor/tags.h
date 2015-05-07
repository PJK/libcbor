/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_TAGS_H
#define LIBCBOR_TAGS_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
* ============================================================================
* Tag manipulation
* ============================================================================
*/

cbor_item_t *cbor_new_tag(uint64_t value);

cbor_item_t *cbor_tag_item(const cbor_item_t *item);

uint64_t cbor_tag_value(const cbor_item_t *item);

void cbor_tag_set_item(cbor_item_t *item, cbor_item_t *tagged_item);

#ifdef __cplusplus
}
#endif

#endif //LIBCBOR_TAGS_H
