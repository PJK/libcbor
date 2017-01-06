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

typedef void(*cbor_load_callback_t)(cJSON *, const struct cbor_callbacks *, void *);

cbor_item_t * cjson_cbor_load(void *source, cbor_load_callback_t cbor_load_callback)
{
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

	/* Context stack */
	struct _cbor_stack stack = _cbor_stack_init();

	/* Target for callbacks */
	struct _cbor_decoder_context context = (struct _cbor_decoder_context) {
		.stack = &stack,
	};
	struct cbor_decoder_result decode_result;

	cbor_load_callback(source, &callbacks, &context);

	return context.root;
}

void cjson_cbor_stream_decode(cJSON * source,
													const struct cbor_callbacks *callbacks,
													void * context)
{
	switch (source->type) {
		case cJSON_False:
		{
			callbacks->boolean(context, false);
			return;
		}
		case cJSON_True:
		{
			callbacks->boolean(context, true);
			return;
		}
		case cJSON_NULL:
		{
			callbacks->null(context);
			return;
		}
		case cJSON_Number:
		{
			callbacks->uint32(context, source->valueint);
			return;
		}
		case cJSON_String:
		{
			callbacks->string(context, (unsigned char *)source->valuestring, strlen(source->valuestring));
			return;
		}
		case cJSON_Array:
		{
			callbacks->array_start(context, cJSON_GetArraySize(source));
			cJSON *item = source->child;
			while (item != NULL) {
				cjson_cbor_stream_decode(item, callbacks, context);
				item = item->next;
			}
			return;
		}
		case cJSON_Object:
		{
			callbacks->map_start(context, cJSON_GetArraySize(source));
			cJSON *item = source->child;
			while (item != NULL) {
				callbacks->string(context, (unsigned char *) item->string, strlen(item->string));
				cjson_cbor_stream_decode(item, callbacks, context);
				item = item->next;
			}
			return;
		}
	}
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
	cbor_item_t * cbor = cjson_cbor_load(json, cjson_cbor_stream_decode);

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