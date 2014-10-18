#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>


unsigned char buffer[512];

static void test_bools(void **state) {
	assert_int_equal(1, cbor_encode_bool(false, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xF4 }), 1);
	assert_int_equal(1, cbor_encode_bool(true, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xF5 }), 1);
}

static void test_null(void **state) {
	assert_int_equal(1, cbor_encode_null(buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xF6 }), 1);
}

static void test_undef(void **state) {
	assert_int_equal(1, cbor_encode_undef(buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xF7 }), 1);
}

int main(void) {
	const UnitTest tests[] = {
		unit_test(test_bools),
		unit_test(test_null),
		unit_test(test_undef)
	};
	return run_tests(tests);
}
