/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include "strings.h"

cbor_item_t *cbor_new_definite_string()
{
	cbor_item_t *item = _CBOR_MALLOC(sizeof(cbor_item_t));
	*item = (cbor_item_t) {
		.refcount = 1,
		.type = CBOR_TYPE_STRING,
		.metadata = {.string_metadata = {_CBOR_METADATA_DEFINITE, 0}}
	};
	return item;
}

cbor_item_t *cbor_new_indefinite_string()
{
	cbor_item_t *item = _CBOR_MALLOC(sizeof(cbor_item_t));
	*item = (cbor_item_t) {
		.refcount = 1,
		.type = CBOR_TYPE_STRING,
		.metadata = {.string_metadata = {.type = _CBOR_METADATA_INDEFINITE, .length = 0}},
		.data = _CBOR_MALLOC(sizeof(struct cbor_indefinite_string_data))
	};
	*((struct cbor_indefinite_string_data *) item->data) = (struct cbor_indefinite_string_data) {
		.chunk_count = 0,
		.chunk_capacity = 0,
		.chunks = NULL,
	};
	return item;
}

cbor_item_t *cbor_build_string(const char *val)
{
	cbor_item_t *item = cbor_new_definite_string();
	size_t len = strlen(val);
	unsigned char * handle = _CBOR_MALLOC(len);
	memcpy(handle, val, len);
	cbor_string_set_handle(item, handle, len);
	return item;
}

void cbor_string_set_handle(cbor_item_t *item, unsigned char *data, size_t length)
{
	assert(cbor_isa_string(item));
	assert(cbor_string_is_definite(item));
	item->data = data;
	item->metadata.string_metadata.length = length;
}

cbor_item_t **cbor_string_chunks_handle(const cbor_item_t *item)
{
	assert(cbor_isa_string(item));
	assert(cbor_string_is_indefinite(item));
	return ((struct cbor_indefinite_string_data *) item->data)->chunks;
}

size_t cbor_string_chunk_count(const cbor_item_t *item)
{
	assert(cbor_isa_string(item));
	assert(cbor_string_is_indefinite(item));
	return ((struct cbor_indefinite_string_data *) item->data)->chunk_count;

}

cbor_item_t *cbor_string_add_chunk(cbor_item_t *item, cbor_item_t *chunk)
{
	assert(cbor_isa_string(item));
	assert(cbor_string_is_indefinite(item));
	struct cbor_indefinite_string_data *data = (struct cbor_indefinite_string_data *) item->data;
	// TODO optimize this with exponential growth
	if (data->chunk_count == data->chunk_capacity) {
		/* We need more space */
		data->chunk_capacity = data->chunk_capacity == 0 ? 1 : data->chunk_capacity * 2;
		cbor_item_t **new_chunks_data =
			_CBOR_REALLOC(data->chunks, data->chunk_capacity * sizeof(cbor_item_t *));
		// TODO handle failures
		data->chunks = new_chunks_data;
	}
	data->chunks[data->chunk_count++] = chunk;
	return item;
}

size_t cbor_string_length(const cbor_item_t *item)
{
	assert(cbor_isa_string(item));
	return item->metadata.string_metadata.length;
}

unsigned char *cbor_string_handle(const cbor_item_t *item)
{
	assert(cbor_isa_string(item));
	return item->data;
}

size_t cbor_string_codepoint_count(const cbor_item_t *item)
{
	assert(cbor_isa_string(item));
	return item->metadata.string_metadata.codepoint_count;
}

bool cbor_string_is_definite(const cbor_item_t *item)
{
	assert(cbor_isa_string(item));
	return item->metadata.string_metadata.type == _CBOR_METADATA_DEFINITE;
}

bool cbor_string_is_indefinite(const cbor_item_t *item)
{
	return !cbor_string_is_definite(item);
}
