/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_CALLBACKS_H
#define LIBCBOR_CALLBACKS_H

#include "cbor/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Callback prototype */
typedef void(*cbor_int8_callback)(void *, uint8_t);

/** Callback prototype */
typedef void(*cbor_int16_callback)(void *, uint16_t);

/** Callback prototype */
typedef void(*cbor_int32_callback)(void *, uint32_t);

/** Callback prototype */
typedef void(*cbor_int64_callback)(void *, uint64_t);

/** Callback prototype */
typedef void(*cbor_simple_callback)(void *);

/** Callback prototype */
typedef void(*cbor_string_callback)(void *, cbor_data, size_t);

/** Callback prototype */
typedef void(*cbor_collection_callback)(void *, size_t);

/** Callback prototype */
typedef void(*cbor_float_callback)(void *, float);

/** Callback prototype */
typedef void(*cbor_double_callback)(void *, double);

/** Callback prototype */
typedef void(*cbor_bool_callback)(void *, bool);

/** Callback bundle -- passed to the decoder */
struct cbor_callbacks {
	/** Unsigned int */
	cbor_int8_callback uint8;
	/** Unsigned int */
	cbor_int16_callback uint16;
	/** Unsigned int */
	cbor_int32_callback uint32;
	/** Unsigned int */
	cbor_int64_callback uint64;

	/** Negative int */
	cbor_int64_callback negint64;
	/** Negative int */
	cbor_int32_callback negint32;
	/** Negative int */
	cbor_int16_callback negint16;
	/** Negative int */
	cbor_int8_callback negint8;

	/** Definite byte string */
	cbor_simple_callback byte_string_start;
	/** Indefinite byte string start */
	cbor_string_callback byte_string;

	/** Definite string */
	cbor_string_callback string;
	/** Indefinite string start */
	cbor_simple_callback string_start;

	/** Definite array */
	cbor_simple_callback indef_array_start;
	/** Indefinite array */
	cbor_collection_callback array_start;

	/** Definite map */
	cbor_simple_callback indef_map_start;
	/** Indefinite map */
	cbor_collection_callback map_start;

	/** Tags */
	cbor_int64_callback tag;

	/** Half float */
	cbor_float_callback float2;
	/** Single float */
	cbor_double_callback float8;
	/** Double float */
	cbor_float_callback float4;
	/** Undef */
	cbor_simple_callback undefined;
	/** Null */
	cbor_simple_callback null;
	/** Bool */
	cbor_bool_callback boolean;

	/** Indefinite item break */
	cbor_simple_callback indef_break;
};

/** Dummy callback implementation - does nothing */
void cbor_null_uint8_callback(void *, uint8_t);

/** Dummy callback implementation - does nothing */
void cbor_null_uint16_callback(void *, uint16_t);

/** Dummy callback implementation - does nothing */
void cbor_null_uint32_callback(void *, uint32_t);

/** Dummy callback implementation - does nothing */
void cbor_null_uint64_callback(void *, uint64_t);

/** Dummy callback implementation - does nothing */
void cbor_null_negint8_callback(void *, uint8_t);

/** Dummy callback implementation - does nothing */
void cbor_null_negint16_callback(void *, uint16_t);

/** Dummy callback implementation - does nothing */
void cbor_null_negint32_callback(void *, uint32_t);

/** Dummy callback implementation - does nothing */
void cbor_null_negint64_callback(void *, uint64_t);

/** Dummy callback implementation - does nothing */
void cbor_null_string_callback(void *, cbor_data, size_t);

/** Dummy callback implementation - does nothing */
void cbor_null_string_start_callback(void *);

/** Dummy callback implementation - does nothing */
void cbor_null_byte_string_callback(void *, cbor_data, size_t);

/** Dummy callback implementation - does nothing */
void cbor_null_byte_string_start_callback(void *);

/** Dummy callback implementation - does nothing */
void cbor_null_array_start_callback(void *, size_t);

/** Dummy callback implementation - does nothing */
void cbor_null_indef_array_start_callback(void *);

/** Dummy callback implementation - does nothing */
void cbor_null_map_start_callback(void *, size_t);

/** Dummy callback implementation - does nothing */
void cbor_null_indef_map_start_callback(void *);

/** Dummy callback implementation - does nothing */
void cbor_null_tag_callback(void *, uint64_t);

/** Dummy callback implementation - does nothing */
void cbor_null_float2_callback(void *, float);

/** Dummy callback implementation - does nothing */
void cbor_null_float4_callback(void *, float);

/** Dummy callback implementation - does nothing */
void cbor_null_float8_callback(void *, double);

/** Dummy callback implementation - does nothing */
void cbor_null_null_callback(void *);

/** Dummy callback implementation - does nothing */
void cbor_null_undefined_callback(void *);

/** Dummy callback implementation - does nothing */
void cbor_null_boolean_callback(void *, bool);

/** Dummy callback implementation - does nothing */
void cbor_null_indef_break_callback(void *);

/** Dummy callback bundle - does nothing */
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

#ifdef __cplusplus
}
#endif


#endif //LIBCBOR_CALLBACKS_H
