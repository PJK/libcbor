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

/* These test verify behavior on interesting randomly generated inputs from the fuzzer */

cbor_item_t *item;
struct cbor_load_result res;

unsigned char data1[] = {0xA9, 0x85};
static void test_1(void **state)
{
	item = cbor_load(data1, 2, &res);
	assert_null(item);
	assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
	assert_int_equal(res.error.position, 2);
}

unsigned char data2[] = {0x9D};
static void test_2(void **state)
{
	item = cbor_load(data2, 1, &res);
	assert_null(item);
	assert_true(res.error.code == CBOR_ERR_MALFORMATED);
	assert_int_equal(res.error.position, 0);
}

unsigned char data3[] = {0xD6};
static void test_3(void **state)
{
	item = cbor_load(data3, 1, &res);
	assert_null(item);
	assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
	assert_int_equal(res.error.position, 1);
}

unsigned char data4[] = {0xBA, 0xC1, 0xE8, 0x3E, 0xE7, 0x20, 0xA8};
static void test_4(void **state)
{
	item = cbor_load(data4, 7, &res);
	assert_null(item);
	assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
	assert_int_equal(res.error.position, 1);
}


int main(void)
{
	const UnitTest tests[] = {
		unit_test(test_1),
		unit_test(test_2),
		unit_test(test_3),
		unit_test(test_4)
	};
	return run_tests(tests);
}
