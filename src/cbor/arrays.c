/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "arrays.h"
#include <assert.h>

size_t cbor_array_size(const cbor_item_t *item)
{
	assert(cbor_isa_array(item));
	return item->metadata.array_metadata.size;
}

cbor_item_t * cbor_array_get(const cbor_item_t * item, size_t index)
{
	cbor_incref(((cbor_item_t **) item->data)[index]);
}

void cbor_array_set(cbor_item_t * item, size_t index, cbor_item_t * value)
{
	((cbor_item_t **) item->data)[index] = cbor_incref(value);

}

void cbor_array_replace(cbor_item_t * item, size_t index, cbor_item_t * value)
{
	/* We cannot use cbor_array_get as that would increase the refcount */
	cbor_intermediate_decref(((cbor_item_t **) item->data)[index]);
	cbor_array_set(item, index, value);
}

cbor_item_t *cbor_array_push(cbor_item_t *array, cbor_item_t *pushee)
{
	assert(cbor_isa_array(array));
	cbor_incref(pushee);
	struct _cbor_array_metadata *metadata = (struct _cbor_array_metadata *) &array->metadata;
	cbor_item_t **data = (cbor_item_t **) array->data;
	if (cbor_array_is_definite(array)) {
		// TODO check size - throw
		data[metadata->ptr++] = pushee;
	} else {
		// TODO exponential reallocs?
		data = _CBOR_REALLOC(data, (metadata->size + 1) * sizeof(cbor_item_t *));
		data[metadata->size++] = pushee;
		array->data = (unsigned char *) data;
	}
	return array;
}


bool cbor_array_is_definite(const cbor_item_t *item)
{
	assert(cbor_isa_array(item));
	return item->metadata.array_metadata.type == _CBOR_METADATA_DEFINITE;
}

bool cbor_array_is_indefinite(const cbor_item_t *item)
{
	assert(cbor_isa_array(item));
	return item->metadata.array_metadata.type == _CBOR_METADATA_INDEFINITE;
}

cbor_item_t **cbor_array_handle(const cbor_item_t *item)
{
	assert(cbor_isa_array(item));
	return (cbor_item_t **) item->data;
}

cbor_item_t *cbor_new_definite_array(size_t size)
{
	cbor_item_t *item = _CBOR_MALLOC(sizeof(cbor_item_t));
	*item = (cbor_item_t) {
		.refcount = 1,
		.type = CBOR_TYPE_ARRAY,
		.metadata = {.array_metadata = {.type = _CBOR_METADATA_DEFINITE, .size = size, .ptr = 0}},
		.data = _CBOR_MALLOC(sizeof(cbor_item_t *) * size)
	};
	return item;
}

cbor_item_t *cbor_new_indefinite_array()
{
	cbor_item_t *item = _CBOR_MALLOC(sizeof(cbor_item_t));
	*item = (cbor_item_t) {
		.refcount = 1,
		.type = CBOR_TYPE_ARRAY,
		.metadata = {.array_metadata = {.type = _CBOR_METADATA_INDEFINITE, .size = 0, .ptr = 0}},
		.data = NULL /* Can be safely realloc-ed */
	};
	return item;
}
