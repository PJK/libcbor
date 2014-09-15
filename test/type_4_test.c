#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>
#include "assertions.h"


cbor_item_t * arr;
struct cbor_load_result res;

unsigned char data1[] = { 0x80, 0xFF };

static void test_empty_array(void **state)
{
	arr = cbor_load(data1, 2, CBOR_FLAGS_NONE, &res);
	assert_non_null(arr);
	assert_true(cbor_typeof(arr) == CBOR_TYPE_ARRAY);
	assert_true(cbor_isa_array(arr));
	assert_true(cbor_array_get_size(arr) == 0);
	assert_true(res.read == 1);
	cbor_decref(&arr);
	assert_null(arr);
}

unsigned char data2[] = { 0x81, 0x01, 0xFF };

static void test_simple_array(void **state)
{
	arr = cbor_load(data2, 3, CBOR_FLAGS_NONE, &res);
	assert_non_null(arr);
	assert_true(cbor_typeof(arr) == CBOR_TYPE_ARRAY);
	assert_true(cbor_isa_array(arr));
	assert_true(cbor_array_get_size(arr) == 1);
	assert_true(res.read == 2);
	/* Check the values */
	assert_uint8(cbor_array_handle(arr)[0], 1);
	cbor_decref(&arr);
	assert_null(arr);
}

unsigned char data3[] = { 0x82, 0x01, 0x81, 0x01, 0xFF };

static void test_nested_arrays(void **state)
{
	arr = cbor_load(data3, 5, CBOR_FLAGS_NONE, &res);
	assert_non_null(arr);
	assert_true(cbor_typeof(arr) == CBOR_TYPE_ARRAY);
	assert_true(cbor_isa_array(arr));
	assert_true(cbor_array_get_size(arr) == 2);
	assert_true(res.read == 4);
	/* Check the values */
	assert_uint8(cbor_array_handle(arr)[0], 1);

	cbor_item_t * nested = cbor_array_handle(arr)[1];
	assert_true(cbor_isa_array(nested));
	assert_true(cbor_array_get_size(nested) == 1);
	assert_uint8(cbor_array_handle(nested)[0], 1);

	cbor_decref(&arr);
	assert_null(arr);
}

int main(void) {
	const UnitTest tests[] = {
		unit_test(test_empty_array),
		unit_test(test_simple_array),
		unit_test(test_nested_arrays)
	};
	return run_tests(tests);
}
