/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_CALLBACKS_H
#define LIBCBOR_CALLBACKS_H

#include "common.h"

enum cbor_callback_result {
	CBOR_CALLBACK_OK,
	CBOR_CALLBACK_SKIP //TODO
};

typedef enum cbor_callback_result(* cbor_int8_callback)(void *, uint8_t);
typedef enum cbor_callback_result(* cbor_int16_callback)(void *, uint16_t);
typedef enum cbor_callback_result(* cbor_int32_callback)(void *, uint32_t);
typedef enum cbor_callback_result(* cbor_int64_callback)(void *, uint64_t);
typedef enum cbor_callback_result(* cbor_simple_callback)(void *);
typedef enum cbor_callback_result(* cbor_string_callback)(void *, cbor_data, size_t);
typedef enum cbor_callback_result(* cbor_collection_callback)(void *, size_t);
typedef enum cbor_callback_result(* cbor_float_callback)(void *, float);
typedef enum cbor_callback_result(* cbor_double_callback)(void *, double);
typedef enum cbor_callback_result(* cbor_bool_callback)(void *, bool);

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
	cbor_float_callback float2; /* 2B float is not supported in standard C */
	cbor_float_callback float4;
	cbor_double_callback float8;
	cbor_simple_callback undefined;
	cbor_simple_callback null; // TODO - other
	cbor_bool_callback boolean;

	/* Shared indefinites */
	cbor_simple_callback indef_break;
};

#endif //LIBCBOR_CALLBACKS_H
