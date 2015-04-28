/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "encoding.h"
#include "internal/encoders.h"

size_t cbor_encode_uint8(uint8_t value, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint8(value, buffer, buffer_size, 0x00);
}

size_t cbor_encode_uint16(uint16_t value, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint16(value, buffer, buffer_size, 0x00);
}

size_t cbor_encode_uint32(uint32_t value, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint32(value, buffer, buffer_size, 0x00);
}

size_t cbor_encode_uint64(uint64_t value, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint64(value, buffer, buffer_size, 0x00);
}

size_t cbor_encode_uint(uint64_t value, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint(value, buffer, buffer_size, 0x00);
}


size_t cbor_encode_negint8(uint8_t value, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint8(value, buffer, buffer_size, 0x20);
}

size_t cbor_encode_negint16(uint16_t value, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint16(value, buffer, buffer_size, 0x20);
}

size_t cbor_encode_negint32(uint32_t value, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint32(value, buffer, buffer_size, 0x20);
}

size_t cbor_encode_negint64(uint64_t value, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint64(value, buffer, buffer_size, 0x20);
}

size_t cbor_encode_negint(uint64_t value, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint(value, buffer, buffer_size, 0x20);
}

size_t cbor_encode_bytestring_start(size_t length, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint((size_t) length, buffer, buffer_size, 0x40);
}

size_t _cbor_encode_byte(uint8_t value, unsigned char *buffer, size_t buffer_size)
{
	if (buffer_size >= 1) {
		buffer[0] = value;
		return 1;
	} else
		return 0;
}

size_t cbor_encode_indef_bytestring_start(unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_byte(0x5F, buffer, buffer_size);
}

size_t cbor_encode_string_start(size_t length, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint((size_t) length, buffer, buffer_size, 0x60);
}

size_t cbor_encode_indef_string_start(unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_byte(0x7F, buffer, buffer_size);
}

size_t cbor_encode_array_start(size_t length, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint((size_t) length, buffer, buffer_size, 0x80);
}

size_t cbor_encode_indef_array_start(unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_byte(0x9F, buffer, buffer_size);
}

size_t cbor_encode_map_start(size_t length, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint((size_t) length, buffer, buffer_size, 0xA0);
}

size_t cbor_encode_indef_map_start(unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_byte(0xBF, buffer, buffer_size);
}

size_t cbor_encode_tag(uint64_t value, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint(value, buffer, buffer_size, 0xC0);
}

size_t cbor_encode_bool(bool value, unsigned char *buffer, size_t buffer_size)
{
	return value ? _cbor_encode_byte(0xF5, buffer, buffer_size) : _cbor_encode_byte(0xF4, buffer, buffer_size);
}

size_t cbor_encode_null(unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_byte(0xF6, buffer, buffer_size);
}

size_t cbor_encode_undef(unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_byte(0xF7, buffer, buffer_size);
}

size_t cbor_encode_half(float value, unsigned char *buffer, size_t buffer_size)
{
	/* Assuming value is normalized */
	uint32_t val = ((union _cbor_float_helper) {.as_float = value}).as_uint;
	uint16_t res;
	uint8_t exp = (val & 0x7F800000) >> 23; /* 0b0111_1111_1000_0000_0000_0000_0000_0000 */
	uint32_t mant = val & 0x7FFFFF; /* 0b0000_0000_0111_1111_1111_1111_1111_1111 */
	if (exp == 0xFF) { /* Infinity or NaNs */
		if (value != value)
			res = (uint16_t) 0x00e700; /* Not IEEE semantics - required by CBOR [s. 3.9] */
		else
			res = (val & 0x80000000) >> 16 | 0x7C00 | (mant ? 1 : 0) << 15;
	} else if (exp == 0x00) { /* Zeroes or subnorms */
		res = (val & 0x80000000) >> 16 | (uint16_t) (mant >> 13);
	} else { /* Normal numbers */
		exp -= 127;
		if (exp > 15)
			return 0; /* No way we can represent magnitude in normalized way */
		else
			res = (val & 0x80000000) >> 16 | ((exp + 15) << 10) | (uint16_t) (mant >> 13);
	}
	return _cbor_encode_uint16(res, buffer, buffer_size, 0xE0);
}

size_t cbor_encode_single(float value, unsigned char *buffer, size_t buffer_size)
{

	return _cbor_encode_uint32(((union _cbor_float_helper) {.as_float = value}).as_uint, buffer, buffer_size, 0xE0);
}

size_t cbor_encode_double(double value, unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_uint64(((union _cbor_double_helper) {.as_double = value}).as_uint, buffer, buffer_size, 0xE0);
}

size_t cbor_encode_break(unsigned char *buffer, size_t buffer_size)
{
	return _cbor_encode_byte(0xFF, buffer, buffer_size);
}

size_t cbor_encode_ctrl(uint8_t value, unsigned char * buffer, size_t buffer_size)
{
	return _cbor_encode_uint8(value, buffer, buffer_size, 0xE0);
}
