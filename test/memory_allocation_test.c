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

// This test simulates cases when malloc unexpectedly fails and leaves a
// possibly partially constructed object behind. It this is especially useful
// in conjunction with the memory correctness check.
//
// WARNING: The test only works with CBOR_CUSTOM_ALLOC

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
	if (malloc_calls >= malloc_calls_expected) {
		// We failed to properly mock a call
		fail();
	}
	if (should_succeed[malloc_calls++]) {
		return malloc(size);
	} else {
		return NULL;
	}
}

#define WITH_MOCK_MALLOC(block, malloc_calls, ...) do { \
        set_mock_malloc(malloc_calls, __VA_ARGS__); \
        block; \
        finalize_mock_malloc(); \
    } while (0)

static void test_int_creation(void **state)
{
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_new_int8()); });
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_new_int16()); });
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_new_int32()); });
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_new_int64()); });
//
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_build_uint8(0xFF)); });
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_build_uint16(0xFF)); });
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_build_uint32(0xFF)); });
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_build_uint64(0xFF)); });
//
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_build_negint8(0xFF)); });
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_build_negint16(0xFF)); });
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_build_negint32(0xFF)); });
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_build_negint64(0xFF)); });
}

static void test_string_creation(void **state)
{
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_new_definite_string()); });
//
//	// Failure allocating the handle
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_new_indefinite_string()); });
	// Failure allocating the chunk data
	WITH_MOCK_MALLOC({
		assert_null(cbor_new_indefinite_string());
	}, 2, true, false);

//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_build_string("Test")); });
//	WITH_MOCK_MALLOC(1, false, { assert_null(cbor_build_stringn("Test", 4)); });
}

int main(void)
{
#if CBOR_CUSTOM_ALLOC
	cbor_set_allocs(instrumented_malloc, realloc, free);

	const struct CMUnitTest tests[] = {
			cmocka_unit_test(test_int_creation),
			cmocka_unit_test(test_string_creation),
	};
#else
	// Can't do anything without a custom allocator
	const struct CMUnitTest tests[] = {};
#endif

	return cmocka_run_group_tests(tests, NULL, NULL);
}

