#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>


cbor_item_t * string;
struct cbor_load_result res;

unsigned char empty_string_data[] = { 0x60 };
static void test_empty_string(void **state) {
	string = cbor_load(empty_string_data, 1, CBOR_FLAGS_NONE, &res);
	assert_non_null(string);
	assert_true(cbor_typeof(string) == CBOR_TYPE_STRING);
	assert_true(cbor_isa_string(string));
	assert_int_equal(cbor_string_length(string), 0);
	assert_int_equal(cbor_string_codepoint_count(string), 0);
	assert_true(res.read == 1);
	cbor_decref(&string);
	assert_null(string);
}


int main(void) {
	const UnitTest tests[] = {
		unit_test(test_empty_string)
	};
	return run_tests(tests);
}
