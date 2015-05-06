/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_CALLBACKS_H
#define LIBCBOR_CALLBACKS_H

#include "common.h"

typedef void(*cbor_int8_callback)(void *, uint8_t);

typedef void(*cbor_int16_callback)(void *, uint16_t);

typedef void(*cbor_int32_callback)(void *, uint32_t);

typedef void(*cbor_int64_callback)(void *, uint64_t);

typedef void(*cbor_simple_callback)(void *);

typedef void(*cbor_string_callback)(void *, cbor_data, size_t);

typedef void(*cbor_collection_callback)(void *, size_t);

typedef void(*cbor_float_callback)(void *, float);

typedef void(*cbor_double_callback)(void *, double);

typedef void(*cbor_bool_callback)(void *, bool);

struct cbor_callbacks {
	/* Type 0 - Unsigned integers */
	cbor_int8_callback uint8;
	cbor_int16_callback uint16;
	cbor_int32_callback uint32;
	cbor_int64_callback uint64;

	/* Type 1 - Negative integers */
	cbor_int8_callback negint8;
	cbor_int16_callback negint16;
	cbor_int32_callback negint32;
	cbor_int64_callback negint64;

	/* Type 2 - Byte strings */
	cbor_string_callback byte_string;
	cbor_simple_callback byte_string_start;

	/* Type 3 - Strings */
	cbor_string_callback string;
	cbor_simple_callback string_start;

	/* Type 4 - Arrays */
	cbor_collection_callback array_start;
	cbor_simple_callback indef_array_start;

	/* Type 5 - Maps */
	cbor_collection_callback map_start;
	cbor_simple_callback indef_map_start;

	/* Type 6 - Tags */
	cbor_int64_callback tag;

	/* Type 7 - Floats & misc */
	/* Type names cannot be member names */
	cbor_float_callback float2;
	/* 2B float is not supported in standard C */
	cbor_float_callback float4;
	cbor_double_callback float8;
	cbor_simple_callback undefined;
	cbor_simple_callback null; // TODO - other
	cbor_bool_callback boolean;

	/* Shared indefinites */
	cbor_simple_callback indef_break;
};


void cbor_null_uint8_callback(void *, uint8_t);

void cbor_null_uint16_callback(void *, uint16_t);

void cbor_null_uint32_callback(void *, uint32_t);

void cbor_null_uint64_callback(void *, uint64_t);

void cbor_null_negint8_callback(void *, uint8_t);

void cbor_null_negint16_callback(void *, uint16_t);

void cbor_null_negint32_callback(void *, uint32_t);

void cbor_null_negint64_callback(void *, uint64_t);

void cbor_null_string_callback(void *, cbor_data, size_t);

void cbor_null_string_start_callback(void *);

void cbor_null_byte_string_callback(void *, cbor_data, size_t);

void cbor_null_byte_string_start_callback(void *);

void cbor_null_array_start_callback(void *, size_t);

void cbor_null_indef_array_start_callback(void *);

void cbor_null_map_start_callback(void *, size_t);

void cbor_null_indef_map_start_callback(void *);

void cbor_null_tag_callback(void *, uint64_t);

void cbor_null_float2_callback(void *, float);

void cbor_null_float4_callback(void *, float);

void cbor_null_float8_callback(void *, double);

void cbor_null_null_callback(void *);

void cbor_null_undefined_callback(void *);

void cbor_null_boolean_callback(void *, bool);

void cbor_null_indef_break_callback(void *);


static const struct cbor_callbacks cbor_empty_callbacks = {
	/* Type 0 - Unsigned integers */
	.uint8 = cbor_null_uint8_callback,
	.uint16 = cbor_null_uint16_callback,
	.uint32 = cbor_null_uint32_callback,
	.uint64 = cbor_null_uint64_callback,

	/* Type 1 - Negative integers */
	.negint8 = cbor_null_negint8_callback,
	.negint16 = cbor_null_negint16_callback,
	.negint32 = cbor_null_negint32_callback,
	.negint64 = cbor_null_negint64_callback,

	/* Type 2 - Byte strings */
	.byte_string = cbor_null_byte_string_callback,
	.byte_string_start = cbor_null_byte_string_start_callback,

	/* Type 3 - Strings */
	.string = cbor_null_string_callback,
	.string_start = cbor_null_string_start_callback,

	/* Type 4 - Arrays */
	.array_start = cbor_null_array_start_callback,
	.indef_array_start = cbor_null_indef_array_start_callback,

	/* Type 5 - Maps */
	.map_start = cbor_null_map_start_callback,
	.indef_map_start = cbor_null_indef_map_start_callback,

	/* Type 6 - Tags */
	.tag = cbor_null_tag_callback,

	/* Type 7 - Floats & misc */
	/* Type names cannot be member names */
	.float2 = cbor_null_float2_callback,
	/* 2B float is not supported in standard C */
	.float4 = cbor_null_float4_callback,
	.float8 = cbor_null_float8_callback,
	.undefined = cbor_null_undefined_callback,
	.null = cbor_null_null_callback,
	.boolean = cbor_null_boolean_callback,

	/* Shared indefinites */
	.indef_break = cbor_null_indef_break_callback,
};

#endif //LIBCBOR_CALLBACKS_H
