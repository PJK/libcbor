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
	struct _cbor_stack stack = _cbor_stack_init();

	/* Target for callbacks */
	struct _cbor_decoder_context context = (struct _cbor_decoder_context){
		.stack = &stack,
		.creation_failed = false
	};
	struct cbor_decoder_result decode_result;
	*result = (struct cbor_load_result) {.read = 0, .error = {.code = CBOR_ERR_NONE}};

	do {
		if (source_size > result->read) { /* Check for overflows */
			decode_result = cbor_stream_decode(
				source + result->read,
				source_size - result->read,
				&callbacks,
				&context);
		} else {
			result->error = (struct cbor_error) {
				.code = CBOR_ERR_NOTENOUGHDATA,
				.position = result->read};
			goto error;
		}

		switch (decode_result.status) {
		case CBOR_DECODER_FINISHED:
			/* Everything OK */
		{
			result->read += decode_result.read;
			break;
		}
		case CBOR_DECODER_NEDATA:
			/* Data length doesn't match MTB expectation */
		{
			result->error.code = CBOR_ERR_NOTENOUGHDATA;
			goto error;
		}
		case CBOR_DECODER_EBUFFER:
			/* Fallthrough */
		case CBOR_DECODER_ERROR:
			/* Reserved/malformated item */
		{
			result->error.code = CBOR_ERR_MALFORMATED;
			goto error;
		}
		}

		if (context.creation_failed) {
			/* Most likely unsuccessful allocation - our callback has failed */
			result->error.code = CBOR_DECODER_MEMERROR;
			goto error;
		}
	} while (stack.size > 0);

	/* Move the result before free */
	cbor_item_t *result_item = context.root;
	return result_item;

	error:
	result->error.position = result->read;
	debug_print("Failed with decoder error %d at %d\n", result->error.code, result->error.position);
	//cbor_describe(stack.top->item, stdout);
	/* Free the stack */
	while (stack.size > 0) {
		cbor_decref(&stack.top->item);
		_cbor_stack_pop(&stack);
	}
	return NULL;
}

#ifdef PRETTY_PRINTER

#include <inttypes.h>
#include <wchar.h>
#include <locale.h>

#define __STDC_FORMAT_MACROS

static int _pow(int b, int ex)
{
	if (ex == 0) return 1;
	int res = b;
	while (--ex > 0) res *= b;
	return res;
}

static void _cbor_nested_describe(cbor_item_t *item, FILE *out, int indent)
{
	setlocale(LC_ALL, "");
	switch (cbor_typeof(item)) {
	case CBOR_TYPE_UINT: {
		fprintf(out, "%*s[CBOR_TYPE_UINT] ", indent, " ");
		fprintf(out, "Width: %dB, ", _pow(2, cbor_int_get_width(item)));
		fprintf(out, "Value: %"PRIu64"\n", cbor_get_int(item));
		break;
	};
	case CBOR_TYPE_NEGINT: {
		fprintf(out, "%*s[CBOR_TYPE_NEGINT] ", indent, " ");
		fprintf(out, "Width: %dB, ", _pow(2, cbor_int_get_width(item)));
		fprintf(out, "Value: -%"PRIu64" -1\n", cbor_get_int(item));
		break;
	};
	case CBOR_TYPE_BYTESTRING: {
		fprintf(out, "%*s[CBOR_TYPE_BYTESTRING] ", indent, " ");
		if (cbor_bytestring_is_indefinite(item)) {
			fprintf(out,
					"Indefinite, with %"PRIuPTR" chunks:\n",
					cbor_bytestring_chunk_count(item));
			for (size_t i = 0; i < cbor_bytestring_chunk_count(item); i++)
				_cbor_nested_describe(
					cbor_bytestring_chunks_handle(item)[i],
					out,
					indent + 4);
		} else {
			fprintf(out,
					"Definite, length %"PRIuPTR"B\n",
					cbor_bytestring_length(item));
		}
		break;
	};
	case CBOR_TYPE_STRING: {
		fprintf(out, "%*s[CBOR_TYPE_STRING] ", indent, " ");
		if (cbor_string_is_indefinite(item)) {
			fprintf(out,
					"Indefinite, with %"PRIuPTR" chunks:\n",
					cbor_string_chunk_count(item));
			for (size_t i = 0; i < cbor_string_chunk_count(item); i++)
				_cbor_nested_describe(
					cbor_string_chunks_handle(item)[i],
					out,
					indent + 4);
		} else {
			fprintf(out,
					"Definite, length %"PRIuPTR"B, %"PRIuPTR" codepoints\n",
					cbor_string_length(item),
					cbor_string_codepoint_count(item));
			/* Careful - this doesnt support multibyte characters! */
			/* Printing those is out of the scope of this demo :) */
			/* libICU is your friend */
			fprintf(out,
					"%*s%*s\n", indent + 4, " ",
					(int) cbor_string_length(item),
					cbor_string_handle(item)
			);
		}
		break;
	};
	case CBOR_TYPE_ARRAY: {
		fprintf(out, "%*s[CBOR_TYPE_ARRAY] ", indent, " ");
		if (cbor_array_is_definite(item)) {
			fprintf(out,
					"Definite, size: %"PRIuPTR"\n",
					cbor_array_size(item));
		} else {
			fprintf(out,
					"Indefinite, size:  %"PRIuPTR"\n",
					cbor_array_size(item));
		}

		for (size_t i = 0; i < cbor_array_size(item); i++)
			_cbor_nested_describe(
				cbor_array_handle(item)[i],
				out,
				indent + 4);
		break;
	};
	case CBOR_TYPE_MAP: {
		fprintf(out, "%*s[CBOR_TYPE_MAP] ", indent, " ");
		if (cbor_map_is_definite(item)) {
			fprintf(out,
					"Definite, size: %"PRIuPTR"\n",
					cbor_map_size(item));
		} else {
			fprintf(out,
					"Indefinite, size:  %"PRIuPTR"\n",
					cbor_map_size(item));
		}

		for (size_t i = 0; i < cbor_map_size(item); i++) {
			_cbor_nested_describe(
				cbor_map_handle(item)[i].key,
				out,
				indent + 4);
			_cbor_nested_describe(
				cbor_map_handle(item)[i].value,
				out,
				indent + 4);
		}
		break;
	};
	case CBOR_TYPE_TAG: {
		fprintf(out, "%*s[CBOR_TYPE_TAG] ", indent, " ");
		fprintf(out, "Value: %"PRIuPTR"\n", cbor_tag_value(item));
		_cbor_nested_describe(cbor_tag_item(item), out, indent + 4);
		break;
	};
	case CBOR_TYPE_FLOAT_CTRL: {
		fprintf(out, "%*s[CBOR_TYPE_FLOAT_CTRL] ", indent, " ");
		if (cbor_float_ctrl_is_ctrl(item)) {
			if (cbor_is_bool(item))
				fprintf(out, "Bool: %s\n", cbor_ctrl_bool(item) ? "true" : "false");
			else if (cbor_is_undef(item))
				fprintf(out, "Undefined\n");
			else if (cbor_is_null(item))
				fprintf(out, "Null\n");
			else
				fprintf(out, "Simple value %d\n", cbor_ctrl_value(item));
		} else {
			fprintf(out, "Width: %dB, ", _pow(2, cbor_float_get_width(item)));
			fprintf(out, "value: %lf\n", cbor_float_get_float(item));
		}
		break;
	};
	}
}

void cbor_describe(cbor_item_t *item, FILE *out)
{
	_cbor_nested_describe(item, out, 0);
}

#endif
