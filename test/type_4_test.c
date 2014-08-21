#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>


cbor_item_t * arr;
struct cbor_load_result res;

unsigned char data1[] = { 0x80, 0xFF };

static void test_empty_array(void **state) {
	arr = cbor_load(data1, 2, 0, &res);
	assert_non_null(arr);
	assert_true(cbor_typeof(arr) == CBOR_TYPE_ARRAY);
	assert_true(cbor_isa_array(arr));
	assert_true(cbor_array_get_size(arr) == 0);
	assert_true(res.read == 1);
	cbor_decref(&arr);
	assert_null(arr);
}

unsigned char data2[] = { 0x81, 0x01, 0xFF };

static void test_simple_array(void **state) {
	arr = cbor_load(data2, 3, 0, &res);
	assert_non_null(arr);
	assert_true(cbor_typeof(arr) == CBOR_TYPE_ARRAY);
	assert_true(cbor_isa_array(arr));
	assert_true(cbor_array_get_size(arr) == 1);
	assert_true(res.read == 2);
	cbor_decref(&arr);
	assert_null(arr);
}

int main(void) {
	const UnitTest tests[] = {
		unit_test(test_empty_array),
		unit_test(test_simple_array)
	};
	return run_tests(tests);
}
