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
#include "assertions.h"

cbor_item_t * item, * copy;


static void test_uints(void **state)
{
	item = cbor_build_uint8(10);
	assert_uint8(copy = cbor_copy(item), 10);
	cbor_decref(&item);
	cbor_decref(&copy);

	item = cbor_build_uint16(10);
	assert_uint16(copy = cbor_copy(item), 10);
	cbor_decref(&item);
	cbor_decref(&copy);

	item = cbor_build_uint32(10);
	assert_uint32(copy = cbor_copy(item), 10);
	cbor_decref(&item);
	cbor_decref(&copy);

	item = cbor_build_uint64(10);
	assert_uint64(copy = cbor_copy(item), 10);
	cbor_decref(&item);
	cbor_decref(&copy);
}

static void test_negints(void **state)
{
	item = cbor_build_negint8(10);
	assert_true(cbor_get_uint8(copy = cbor_copy(item)) == 10);
	cbor_decref(&item);
	cbor_decref(&copy);

	item = cbor_build_negint16(10);
	assert_true(cbor_get_uint16(copy = cbor_copy(item)) == 10);
	cbor_decref(&item);
	cbor_decref(&copy);

	item = cbor_build_negint32(10);
	assert_true(cbor_get_uint32(copy = cbor_copy(item)) == 10);
	cbor_decref(&item);
	cbor_decref(&copy);

	item = cbor_build_negint64(10);
	assert_true(cbor_get_uint64(copy = cbor_copy(item)) == 10);
	cbor_decref(&item);
	cbor_decref(&copy);
}

static void test_def_bytestring(void **state)
{
	item = cbor_build_bytestring((cbor_data) "abc", 3);
	assert_memory_equal(
		cbor_bytestring_handle(copy = cbor_copy(item)),
		cbor_bytestring_handle(item),
		3
	);
	cbor_decref(&item);
	cbor_decref(&copy);
}

static void test_indef_bytestring(void **state)
{
	item = cbor_new_indefinite_bytestring();
	cbor_bytestring_add_chunk(
		item,
		cbor_move(cbor_build_bytestring((cbor_data) "abc", 3))
	);
	copy = cbor_copy(item);

	assert_int_equal(cbor_bytestring_chunk_count(item), cbor_bytestring_chunk_count(copy));

	assert_memory_equal(
		cbor_bytestring_handle(cbor_bytestring_chunks_handle(copy)[0]),
		"abc",
		3
	);
	cbor_decref(&item);
	cbor_decref(&copy);
}

static void test_def_string(void **state)
{
	item = cbor_build_string("abc");
	assert_memory_equal(
		cbor_string_handle(copy = cbor_copy(item)),
		cbor_string_handle(item),
		3
	);
	cbor_decref(&item);
	cbor_decref(&copy);
}

static void test_indef_string(void **state)
{
	item = cbor_new_indefinite_string();
	cbor_string_add_chunk(
		item,
		cbor_move(cbor_build_string("abc"))
	);
	copy = cbor_copy(item);

	assert_int_equal(cbor_string_chunk_count(item), cbor_string_chunk_count(copy));

	assert_memory_equal(
		cbor_string_handle(cbor_string_chunks_handle(copy)[0]),
		"abc",
		3
	);
	cbor_decref(&item);
	cbor_decref(&copy);
}

static void test_def_array(void **state)
{
	item = cbor_new_definite_array(1);
	cbor_array_push(item, cbor_move(cbor_build_uint8(42)));

	assert_uint8(
		cbor_array_get(copy = cbor_copy(item), 0),
		42
	);
	cbor_decref(&item);
	cbor_decref(&copy);
}

static void test_indef_array(void **state)
{
	item = cbor_new_indefinite_array();
	cbor_array_push(item, cbor_move(cbor_build_uint8(42)));

	assert_uint8(
		cbor_array_get(copy = cbor_copy(item), 0),
		42
	);
	cbor_decref(&item);
	cbor_decref(&copy);
}

static void test_def_map(void **state)
{
	item = cbor_new_definite_map(1);
	cbor_map_add(item, (struct cbor_pair) {
		.key = cbor_move(cbor_build_uint8(42)),
		.value = cbor_move(cbor_build_uint8(43)),
	});

	assert_uint8(
		cbor_map_handle(copy = cbor_copy(item))[0].key,
		42
	);

	cbor_decref(&item);
	cbor_decref(&copy);
}

static void test_indef_map(void **state)
{
	item = cbor_new_indefinite_map(1);
	cbor_map_add(item, (struct cbor_pair) {
		.key = cbor_move(cbor_build_uint8(42)),
		.value = cbor_move(cbor_build_uint8(43)),
	});

	assert_uint8(
		cbor_map_handle(copy = cbor_copy(item))[0].key,
		42
	);

	cbor_decref(&item);
	cbor_decref(&copy);
}

static void test_tag(void **state)
{
	item = cbor_build_tag(10, cbor_move(cbor_build_uint8(42)));

	assert_uint8(
		cbor_tag_item(copy = cbor_copy(item)),
		42
	);

	cbor_decref(&item);
	cbor_decref(&copy);
}


int main(void)
{
	const UnitTest tests[] = {

		unit_test(test_uints),
		unit_test(test_negints),
		unit_test(test_def_bytestring),
		unit_test(test_indef_bytestring),
		unit_test(test_def_string),
		unit_test(test_indef_string),
		unit_test(test_def_array),
		unit_test(test_indef_array),
		unit_test(test_def_map),
		unit_test(test_indef_map),
		unit_test(test_tag)
	};
	return run_tests(tests);
}
