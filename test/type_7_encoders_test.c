#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>
#include <math.h>


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

static void test_break(void **state) {
	assert_int_equal(1, cbor_encode_break(buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xFF }), 1);
}

static void test_half(void **state) {
	assert_int_equal(3, cbor_encode_half(1.5, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xF9, 0x3E, 0x00 }), 3);
	assert_int_equal(3, cbor_encode_half(-0.0f, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xF9, 0x80, 0x00 }), 3);
	assert_int_equal(3, cbor_encode_half(0.0f, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xF9, 0x00, 0x00 }), 3);
	assert_int_equal(3, cbor_encode_half(65504.0, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xF9, 0x7B, 0xFF }), 3);
	assert_int_equal(3, cbor_encode_half(INFINITY, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xF9, 0x7C, 0x00 }), 3);
}

static void test_half_special(void **state)
{
	assert_int_equal(3, cbor_encode_half(NAN, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xF9, 0xE7, 0x00 }), 3);
}

static void test_float(void **state) {
	assert_int_equal(5, cbor_encode_float(3.4028234663852886e+38, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xFA, 0x7F, 0x7F, 0xFF, 0xFF }), 5);
}

static void test_double(void **state) {
	assert_int_equal(9, cbor_encode_double(1.0e+300, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xFB, 0x7E, 0x37, 0xE4, 0x3C, 0x88, 0x00, 0x75, 0x9C }), 9);
}

int main(void) {
	const UnitTest tests[] = {
		unit_test(test_bools),
		unit_test(test_null),
		unit_test(test_undef),
		unit_test(test_break),
		unit_test(test_half),
		unit_test(test_float),
		unit_test(test_double),
		unit_test(test_half_special)
	};
	return run_tests(tests);
}
