/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include "arrays.h"

size_t cbor_array_size(const cbor_item_t *item)
{
	assert(cbor_isa_array(item));
	return item->metadata.array_metadata.end_ptr;
}

cbor_item_t * cbor_array_get(const cbor_item_t * item, size_t index)
{
	return cbor_incref(((cbor_item_t **) item->data)[index]);
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
		if (metadata->end_ptr > metadata->allocated)
			printf("Error - NE space def\n");
		data[metadata->end_ptr++] = pushee;
	} else {
		// TODO exponential reallocs?
		if (metadata->end_ptr > metadata->allocated)
			printf("Error - NE space indef\n");
		data = _CBOR_REALLOC(data, (++metadata->allocated) * sizeof(cbor_item_t *));
		data[metadata->end_ptr++] = pushee;
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
	if (item == NULL)
		return NULL;

	cbor_item_t ** data = _CBOR_MALLOC(sizeof(cbor_item_t *) * size);
	if (data == NULL) {
		_CBOR_FREE(item);
		return NULL;
	}

	for (size_t i = 0; i < size; i++)
		data[i] = NULL;

	*item = (cbor_item_t) {
		.refcount = 1,
		.type = CBOR_TYPE_ARRAY,
		.metadata = {
			.array_metadata = {
				.type = _CBOR_METADATA_DEFINITE,
				.allocated = size, .end_ptr = 0
			}
		},
		.data = (unsigned char *)data
	};

	return item;
}

cbor_item_t *cbor_new_indefinite_array()
{
	cbor_item_t *item = _CBOR_MALLOC(sizeof(cbor_item_t));
	if (item == NULL)
		return NULL;

	*item = (cbor_item_t) {
		.refcount = 1,
		.type = CBOR_TYPE_ARRAY,
		.metadata = {
			.array_metadata = {
				.type = _CBOR_METADATA_INDEFINITE,
				.allocated = 0,
				.end_ptr = 0
			}
		},
		.data = NULL /* Can be safely realloc-ed */
	};
	return item;
}
