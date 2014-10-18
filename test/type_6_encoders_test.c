#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>


unsigned char buffer[512];

static void test_embedded_tag(void **state) {
	assert_int_equal(1, cbor_encode_tag(1, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xC1 }), 1);
}

static void test_tag(void **state) {
	assert_int_equal(5, cbor_encode_tag(1000000, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0xDA, 0x00, 0x0F, 0x42, 0x40 }), 5);
}

int main(void) {
	const UnitTest tests[] = {
		unit_test(test_embedded_tag),
		unit_test(test_tag),
	};
	return run_tests(tests);
}
