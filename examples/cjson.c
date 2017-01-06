/*
 * Contributed by Jacob Teplitsky <jacob.teplitsky@ericsson.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */


/**
 * This code demonstrates how cJSON (https://github.com/DaveGamble/cJSON) callbacks
 * can be used in conjuction with the streaming parser to translate JSON to CBOR.
 *
 * The example will only be compiled when cJSON is available
 */

#include <string.h>
#include <cjson/cJSON.h>
#include "cbor.h"
#include "cbor/internal/builder_callbacks.h"
#include "cbor/internal/loaders.h"

typedef struct cbor_decoder_result(*cbor_load_callback_t)(void *, size_t,  const struct cbor_callbacks *, void *);

cbor_item_t *cjson_cbor_load(void *source,
							 size_t source_size,
							 struct cbor_load_result *result,
							 cbor_load_callback_t cbor_load_callback)
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
	struct _cbor_decoder_context context = (struct _cbor_decoder_context) {
		.stack = &stack,
		.creation_failed = false,
		.syntax_error = false
	};
	struct cbor_decoder_result decode_result;
	*result = (struct cbor_load_result) {.read = 0, .error = {.code = CBOR_ERR_NONE}};

	do {
		if (source_size > result->read) { /* Check for overflows */
			decode_result = cbor_load_callback(
					source, 24,
					&callbacks,
					&context
			);
		} else {
			result->error = (struct cbor_error) {
					.code = CBOR_ERR_NOTENOUGHDATA,
					.position = result->read
			};
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
			result->error.code = CBOR_ERR_MEMERROR;
			goto error;
		} else if (context.syntax_error) {
			result->error.code = CBOR_ERR_SYNTAXERROR;
			goto error;
		}
	} while (stack.size > 0);

	/* Move the result before free */
	cbor_item_t *result_item = context.root;
	return result_item;

	error:
	result->error.position = result->read;
	//debug_print("Failed with decoder error %d at %d\n", result->error.code, result->error.position);
	//cbor_describe(stack.top->item, stdout);
	/* Free the stack */
	while (stack.size > 0) {
		cbor_decref(&stack.top->item);
		_cbor_stack_pop(&stack);
	}
	return NULL;
}

struct cbor_decoder_result cjson_cbor_stream_decode(void *source_void, size_t source_size,
													const struct cbor_callbacks *callbacks,
													void *context)
{
	cJSON *source = source_void;
	/* If we have no data, we cannot read even the MTB */
	if (source_size < 1) {
		return (struct cbor_decoder_result) {0, CBOR_DECODER_EBUFFER};
	}

	/* If we have a byte, assume it's the MTB */
	struct cbor_decoder_result result = {1, CBOR_DECODER_FINISHED};

	switch (source->type) {
		case cJSON_False:
		{
			callbacks->boolean(context, false);
			return result;
		}
		case cJSON_True:
		{
			callbacks->boolean(context, false);
			return result;
		}
		case cJSON_NULL:
		{
			callbacks->null(context);
			return result;
		}
		case cJSON_Number:
		{
			callbacks->uint32(context, source->valueint);
			return result;
		}
		case cJSON_String:
		{
			callbacks->string(context, (unsigned char *)source->valuestring, strlen(source->valuestring));
			return result;
		}
		case cJSON_Array:
		{
			callbacks->array_start(context, cJSON_GetArraySize(source));
			cJSON *item = source->child;
			while (item != NULL) {
				cjson_cbor_stream_decode(item, 2, callbacks, context);
				item = item->next;
			}

			return result;
		}
		case cJSON_Object:
		{
			callbacks->map_start(context, cJSON_GetArraySize(source));
			cJSON *item = source->child;
			while (item != NULL) {
				callbacks->string(context, (unsigned char *) item->string, strlen(item->string));
				cjson_cbor_stream_decode(item, 2, callbacks, context);
				item = item->next;
			}

			return result;
		}
		default: /* Never happens - this shuts up the compiler */
		{
			return result;
		}
	}
}

cbor_item_t * cjson_to_cbor(cJSON *json)
{
	struct cbor_load_result result;
	return cjson_cbor_load(json, 10, &result, cjson_cbor_stream_decode);
}

void usage()
{
	printf("Usage: cjson [input JSON file]\n");
	exit(1);
}

int main(int argc, char * argv[])
{
	if (argc != 2)
		usage();
	FILE * f = fopen(argv[1], "rb");
	if (f == NULL)
		usage();
	/* Read input file into a buffer (cJSON doesn't work with streams) */
	fseek(f, 0, SEEK_END);
	size_t length = (size_t)ftell(f);
	fseek(f, 0, SEEK_SET);
	char * json_buffer = malloc(length + 1);
	fread(json_buffer, length, 1, f);
	json_buffer[length] = '\0';

	/* Convert between JSON and CBOR */
	cJSON * json = cJSON_Parse(json_buffer);
	cbor_item_t * cbor = cjson_to_cbor(json);

	/* Print out CBOR bytes */
	unsigned char * buffer;
	size_t buffer_size,
			cbor_length = cbor_serialize_alloc(cbor, &buffer, &buffer_size);

	fwrite(buffer, 1, cbor_length, stdout);

	free(buffer);
	fflush(stdout);
	cJSON_Delete(json);
	cbor_decref(&cbor);

}