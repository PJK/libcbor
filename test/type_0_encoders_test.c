#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>


unsigned char buffer[512];

static void test_embedded_uint8(void **state) {
	cbor_encode_uint8(14, buffer, 512);
	assert_memory_equal(buffer, (unsigned char[]){ 0x0E }, 1);
}

static void test_uint16(void **state) {
	cbor_encode_uint16(1000, buffer, 512);
	assert_memory_equal(buffer, ((unsigned char[]){ 0x19, 0x03, 0xE8 }), 3);
}

int main(void) {
	const UnitTest tests[] = {
		unit_test(test_embedded_uint8),
		unit_test(test_uint16)
	};
	return run_tests(tests);
}
