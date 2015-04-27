/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_BUILDER_CALLBACKS_H
#define LIBCBOR_BUILDER_CALLBACKS_H

#include "../common.h"
#include "../callbacks.h"
#include "stack.h"

struct _cbor_decoder_context {
	cbor_item_t *root;
	struct _cbor_stack *stack;
};

enum cbor_callback_result cbor_builder_uint8_callback(void *, uint8_t);

enum cbor_callback_result cbor_builder_uint16_callback(void *, uint16_t);

enum cbor_callback_result cbor_builder_uint32_callback(void *, uint32_t);

enum cbor_callback_result cbor_builder_uint64_callback(void *, uint64_t);

enum cbor_callback_result cbor_builder_negint8_callback(void *, uint8_t);

enum cbor_callback_result cbor_builder_negint16_callback(void *, uint16_t);

enum cbor_callback_result cbor_builder_negint32_callback(void *, uint32_t);

enum cbor_callback_result cbor_builder_negint64_callback(void *, uint64_t);

enum cbor_callback_result cbor_builder_string_callback(void *, cbor_data, size_t);

enum cbor_callback_result cbor_builder_string_start_callback(void *);

enum cbor_callback_result cbor_builder_byte_string_callback(void *, cbor_data, size_t);

enum cbor_callback_result cbor_builder_byte_string_start_callback(void *);

enum cbor_callback_result cbor_builder_array_start_callback(void *, size_t);

enum cbor_callback_result cbor_builder_indef_array_start_callback(void *);

enum cbor_callback_result cbor_builder_map_start_callback(void *, size_t);

enum cbor_callback_result cbor_builder_indef_map_start_callback(void *);

enum cbor_callback_result cbor_builder_tag_callback(void *, uint64_t);

enum cbor_callback_result cbor_builder_float2_callback(void *, float);

enum cbor_callback_result cbor_builder_float4_callback(void *, float);

enum cbor_callback_result cbor_builder_float8_callback(void *, double);

enum cbor_callback_result cbor_builder_null_callback(void *);

enum cbor_callback_result cbor_builder_undefined_callback(void *);

enum cbor_callback_result cbor_builder_boolean_callback(void *, bool);

enum cbor_callback_result cbor_builder_indef_break_callback(void *);

#endif //LIBCBOR_BUILDER_CALLBACKS_H
