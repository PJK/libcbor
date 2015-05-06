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

int main(void)
{
	const UnitTest tests[] = {
		unit_test(test_1)
	};
	return run_tests(tests);
}
