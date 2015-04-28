/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "cbor.h"
#include "cbor/internal/builder_callbacks.h"
#include "cbor/internal/loaders.h"

cbor_item_t *cbor_load(cbor_data source,
                       size_t source_size,
                       struct cbor_load_result *result)
{
	/* Context stack */
	static struct cbor_callbacks callbacks = {
		.uint8 = &cbor_builder_uint8_callback,
		.uint16 = &cbor_builder_uint16_callback,
		.uint32 = &cbor_builder_uint32_callback,
		.uint64 = &cbor_builder_uint64_callback,

		.negint8 = &cbor_builder_negint8_callback,
		.negint16 = &cbor_builder_negint16_callback,
		.negint32 = &cbor_builder_negint32_callback,
		.negint64 = &cbor_builder_negint64_callback,

		.byte_string = &cbor_builder_byte_string_callback,
		.byte_string_start = &cbor_builder_byte_string_start_callback,

		.string = &cbor_builder_string_callback,
		.string_start = &cbor_builder_string_start_callback,

		.array_start = &cbor_builder_array_start_callback,
		.indef_array_start = &cbor_builder_indef_array_start_callback,

		.map_start = &cbor_builder_map_start_callback,
		.indef_map_start = &cbor_builder_indef_map_start_callback,

		.tag = &cbor_builder_tag_callback,

		.null = &cbor_builder_null_callback,
		.undefined = &cbor_builder_undefined_callback,
		.boolean = &cbor_builder_boolean_callback,
		.float2 = &cbor_builder_float2_callback,
		.float4 = &cbor_builder_float4_callback,
		.float8 = &cbor_builder_float8_callback,
		.indef_break = &cbor_builder_indef_break_callback
	};

	if (source_size == 0) {
		result->error.code = CBOR_ERR_NODATA;
		return NULL;
	}
	/* Target for callbacks */
	struct _cbor_decoder_context *context = _CBOR_MALLOC(sizeof(struct _cbor_decoder_context));
	struct _cbor_stack stack = _cbor_stack_init();
	context->stack = &stack;
	struct cbor_decoder_result decode_result;
	*result = (struct cbor_load_result) {.read = 0, .error = {.code = CBOR_ERR_NONE}};

	do {
		if (source_size > result->read) { /* Check for overflows */
			decode_result = cbor_stream_decode(source + result->read, source_size - result->read, &callbacks, context);
		} else
			goto error;

		if (decode_result.status == CBOR_DECODER_FINISHED) {
			result->read += decode_result.read;
		} else
			goto error;
	} while (stack.size > 0);

	/* Move the result before free */
	cbor_item_t *result_item = context->root;
	_CBOR_FREE(context);
	return result_item;

	error:
	/* Free the stack */
	while (stack.size > 0) {
		cbor_decref(&stack.top->item);
		_cbor_stack_pop(&stack);
	}
	_CBOR_FREE(context);
	result->error.code = CBOR_ERR_NOTENOUGHDATA;
	return NULL;
}
