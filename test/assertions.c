/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "assertions.h"

void assert_uint8(cbor_item_t * item, uint8_t num)
{
	assert_true(cbor_isa_uint(item));
	assert_true(cbor_int_get_width(item) == CBOR_INT_8);
	assert_true(cbor_get_uint8(item) == num);
}

void assert_decoder_result(size_t read, enum cbor_decoder_status status, struct cbor_decoder_result result)
{
	assert_true(read == result.read);
	assert_true(status == result.status);
}
