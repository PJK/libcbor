/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "maps.h"

size_t cbor_map_size(const cbor_item_t *item)
{
	assert(cbor_isa_map(item));
	return item->metadata.map_metadata.end_ptr;
}

size_t cbor_map_allocated(const cbor_item_t *item)
{
	assert(cbor_isa_map(item));
	return item->metadata.map_metadata.allocated;
}

cbor_item_t *cbor_new_definite_map(size_t size)
{
	cbor_item_t *item = _CBOR_MALLOC(sizeof(cbor_item_t));
	if (item == NULL)
		return NULL;
	*item = (cbor_item_t) {
		.refcount = 1,
		.type = CBOR_TYPE_MAP,
		.metadata = {.map_metadata = {
			.allocated = size,
			.type = _CBOR_METADATA_DEFINITE,
			.end_ptr = 0
		}},
		.data = _CBOR_MALLOC(sizeof(struct cbor_pair) * size)
	};
	if (item->data == NULL) {
		_CBOR_FREE(item);
		return NULL;
	}
	return item;
}

cbor_item_t *cbor_new_indefinite_map()
{
	cbor_item_t *item = _CBOR_MALLOC(sizeof(cbor_item_t));
	if (item == NULL)
		return NULL;
	*item = (cbor_item_t) {
		.refcount = 1,
		.type = CBOR_TYPE_MAP,
		.metadata = {.map_metadata = {
			.allocated = 0,
			.type = _CBOR_METADATA_INDEFINITE,
			.end_ptr = 0
		}},
		.data = NULL
	};

	return item;
}


bool _cbor_map_add_key(cbor_item_t *item, cbor_item_t *key)
{
	assert(cbor_isa_map(item));
	struct _cbor_map_metadata *metadata = (struct _cbor_map_metadata *) &item->metadata;
	if (cbor_map_is_definite(item)) {
		struct cbor_pair *data = cbor_map_handle(item);
		if (metadata->end_ptr >= metadata->allocated)
			/* Don't realloc definite preallocated map */
			return false;
		data[metadata->end_ptr].key = key;
		data[metadata->end_ptr++].value = NULL;
	} else {
		if (metadata->end_ptr >= metadata->allocated) {
			/* Exponential realloc */
			size_t new_allocation = (size_t)(CBOR_BUFFER_GROWTH * (metadata->allocated));
			new_allocation = new_allocation ? new_allocation : 1;
			unsigned char * new_data = _CBOR_REALLOC(item->data,
													 new_allocation * sizeof(struct cbor_pair));
			if (new_data == NULL)
				return false;
			item->data = new_data;
			metadata->allocated = new_allocation;
		}
		struct cbor_pair *data = cbor_map_handle(item);
		data[metadata->end_ptr].key = key;
		data[metadata->end_ptr++].value = NULL;
	}
	cbor_incref(key);
	return true;
}

bool _cbor_map_add_value(cbor_item_t *item, cbor_item_t *value)
{
	assert(cbor_isa_map(item));
	cbor_incref(value);
	cbor_map_handle(item)[
		/* Move one back since we are assuming _add_key (which increased the ptr)
		 * was the previous operation on this object */
		item->metadata.map_metadata.end_ptr - 1
	].value = value;
	return true;
}

bool cbor_map_add(cbor_item_t *item, struct cbor_pair pair)
{
	assert(cbor_isa_map(item));
	if (!_cbor_map_add_key(item, pair.key))
		return false;
	return _cbor_map_add_value(item, pair.value);
}

bool cbor_map_is_definite(const cbor_item_t *item)
{
	assert(cbor_isa_map(item));
	return item->metadata.map_metadata.type == _CBOR_METADATA_DEFINITE;
}

bool cbor_map_is_indefinite(const cbor_item_t *item)
{
	return !cbor_map_is_definite(item);
}

struct cbor_pair *cbor_map_handle(const cbor_item_t *item)
{
	assert(cbor_isa_map(item));
	return (struct cbor_pair *) item->data;
}
