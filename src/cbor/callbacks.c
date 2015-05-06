/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "callbacks.h"

#define CBOR_DUMMY_CALLBACK { }

void cbor_null_uint8_callback(void *_ctx, uint8_t _val) CBOR_DUMMY_CALLBACK

void cbor_null_uint16_callback(void *_ctx, uint16_t _val) CBOR_DUMMY_CALLBACK

void cbor_null_uint32_callback(void *_ctx, uint32_t _val) CBOR_DUMMY_CALLBACK

void cbor_null_uint64_callback(void *_ctx, uint64_t _val) CBOR_DUMMY_CALLBACK

void cbor_null_negint8_callback(void *_ctx, uint8_t _val) CBOR_DUMMY_CALLBACK

void cbor_null_negint16_callback(void *_ctx, uint16_t _val) CBOR_DUMMY_CALLBACK

void cbor_null_negint32_callback(void *_ctx, uint32_t _val) CBOR_DUMMY_CALLBACK

void cbor_null_negint64_callback(void *_ctx, uint64_t _val) CBOR_DUMMY_CALLBACK

void cbor_null_string_callback(void *_ctx, cbor_data _val, size_t _val2) CBOR_DUMMY_CALLBACK

void cbor_null_string_start_callback(void *_ctx) CBOR_DUMMY_CALLBACK

void cbor_null_byte_string_callback(void *_ctx, cbor_data _val, size_t _val2) CBOR_DUMMY_CALLBACK

void cbor_null_byte_string_start_callback(void *_ctx) CBOR_DUMMY_CALLBACK

void cbor_null_array_start_callback(void *_ctx, size_t _val) CBOR_DUMMY_CALLBACK

void cbor_null_indef_array_start_callback(void *_ctx) CBOR_DUMMY_CALLBACK

void cbor_null_map_start_callback(void *_ctx, size_t _val) CBOR_DUMMY_CALLBACK

void cbor_null_indef_map_start_callback(void *_ctx) CBOR_DUMMY_CALLBACK

void cbor_null_tag_callback(void *_ctx, uint64_t _val) CBOR_DUMMY_CALLBACK

void cbor_null_float2_callback(void *_ctx, float _val) CBOR_DUMMY_CALLBACK

void cbor_null_float4_callback(void *_ctx, float _val) CBOR_DUMMY_CALLBACK

void cbor_null_float8_callback(void *_ctx, double _val) CBOR_DUMMY_CALLBACK

void cbor_null_null_callback(void *_ctx) CBOR_DUMMY_CALLBACK

void cbor_null_undefined_callback(void *_ctx) CBOR_DUMMY_CALLBACK

void cbor_null_boolean_callback(void *_ctx, bool _val) CBOR_DUMMY_CALLBACK

void cbor_null_indef_break_callback(void *_ctx) CBOR_DUMMY_CALLBACK
