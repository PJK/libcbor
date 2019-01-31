/*
 * Copyright (c) 2014-2019 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdbool.h>
#include "cbor.h"
#include <inttypes.h>
#include <stdlib.h>

// How many malloc calls we expect
int malloc_calls_expected;
// How many malloc calls we got
int malloc_calls;
// Array of booleans indicating whether to return a block or fail with NULL
bool *should_succeed;

void set_mock_malloc(int calls, ...)
{
	va_list args;
	va_start(args, calls);
	malloc_calls_expected = calls;
	malloc_calls = 0;
	should_succeed = malloc(calls * sizeof(bool));
	for (int i = 0; i < calls; i++) {
		// Promotable types, baby
		should_succeed[i] = (bool) va_arg(args, int);
	}
	va_end(args);
}

void finalize_mock_malloc()
{
	assert_int_equal(malloc_calls, malloc_calls_expected);
	free(should_succeed);
}

void *instrumented_malloc(size_t size)
{
	if (should_succeed[malloc_calls++]) {
		return malloc(size);
	} else {
		return NULL;
	}
}

void test_constructor(cbor_item_t* (*constructor)())
{
	set_mock_malloc(1, false);
	assert_null(constructor());
	finalize_mock_malloc();

}

#define TEST_BUILDER(builder, value) do { \
		set_mock_malloc(1, false); \
		assert_null(builder(value)); \
		finalize_mock_malloc(); \
	} while (0)

static void test_int_creation(void **state)
{
	test_constructor(cbor_new_int8);
	test_constructor(cbor_new_int16);
	test_constructor(cbor_new_int32);
	test_constructor(cbor_new_int64);

	TEST_BUILDER(cbor_build_uint8, 0xFF);
	TEST_BUILDER(cbor_build_uint16, 0xFF);
	TEST_BUILDER(cbor_build_uint32, 0xFF);
	TEST_BUILDER(cbor_build_uint64, 0xFF);

	TEST_BUILDER(cbor_build_negint8, 0xFF);
	TEST_BUILDER(cbor_build_negint16, 0xFF);
	TEST_BUILDER(cbor_build_negint32, 0xFF);
	TEST_BUILDER(cbor_build_negint64, 0xFF);
}

int main(void)
{
#if CBOR_CUSTOM_ALLOC
	cbor_set_allocs(instrumented_malloc, realloc, free);

	const struct CMUnitTest tests[] = {
			cmocka_unit_test(test_int_creation),
	};
#else
	// Can't do anything without a custom allocator

	const struct CMUnitTest tests[] = {};
#endif

	return cmocka_run_group_tests(tests, NULL, NULL);
}

