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

unsigned char short_string_data[] = { 0x6C, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x21 };
/*                              0x60 + 12 | Hello world! */
static void test_short_string(void **state) {
	string = cbor_load(short_string_data, 13, CBOR_FLAGS_NONE, &res);
	assert_non_null(string);
	assert_true(cbor_typeof(string) == CBOR_TYPE_STRING);
	assert_true(cbor_isa_string(string));
	assert_int_equal(cbor_string_length(string), 12);
	assert_int_equal(cbor_string_codepoint_count(string), 12);
	assert_memory_equal(&"Hello world!", cbor_string_handle(string), 12);
	assert_true(res.read == 13);
	cbor_decref(&string);
	assert_null(string);
}


unsigned char short_multibyte_string_data[] = { 0x6F, 0xC4, 0x8C, 0x61, 0x75, 0x65, 0x73, 0x20, 0xC3, 0x9F, 0x76, 0xC4, 0x9B, 0x74, 0x65, 0x21 };
/*                              0x60 + 15 | Čaues ßvěte! */
static void test_short_multibyte_string(void **state) {
	string = cbor_load(short_multibyte_string_data, 16, CBOR_FLAGS_NONE, &res);
	assert_non_null(string);
	assert_true(cbor_typeof(string) == CBOR_TYPE_STRING);
	assert_true(cbor_isa_string(string));
	assert_int_equal(cbor_string_length(string), 15);
	assert_int_equal(cbor_string_codepoint_count(string), 12);
	assert_memory_equal(&"Čaues ßvěte!", cbor_string_handle(string), 15);
	assert_true(res.read == 16);
	cbor_decref(&string);
	assert_null(string);
}

int main(void) {
	const UnitTest tests[] = {
		unit_test(test_empty_string),
		unit_test(test_short_string),
		unit_test(test_short_multibyte_string)
	};
	return run_tests(tests);
}
