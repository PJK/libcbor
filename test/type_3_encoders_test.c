/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>


unsigned char buffer[512];

static void test_embedded_string_start(void **state)
{
	assert_int_equal(1, cbor_encode_string_start(1, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]) {0x61}), 1);
}

static void test_string_start(void **state)
{
	assert_int_equal(5, cbor_encode_string_start(1000000, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]) {0x7A, 0x00, 0x0F, 0x42, 0x40}), 5);
}

static void test_indef_string_start(void **state)
{
	assert_int_equal(1, cbor_encode_indef_string_start(buffer, 512));
	assert_int_equal(0, cbor_encode_indef_string_start(buffer, 0));
	assert_memory_equal(buffer, ((unsigned char[]) {0x7F}), 1);
}

int main(void)
{
	const UnitTest tests[] = {
		unit_test(test_embedded_string_start),
		unit_test(test_string_start),
		unit_test(test_indef_string_start)
	};
	return run_tests(tests);
}
