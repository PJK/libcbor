#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>


unsigned char buffer[512];

static void test_embedded_bytestring_start(void **state)
{
	assert_int_equal(1, cbor_encode_bytestring_start(1, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]) {0x41}), 1);
}

static void test_bytestring_start(void **state)
{
	assert_int_equal(5, cbor_encode_bytestring_start(1000000, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]) {0x5A, 0x00, 0x0F, 0x42, 0x40}), 5);
}

static void test_indef_bytestring_start(void **state)
{
	assert_int_equal(0, cbor_encode_indef_bytestring_start(buffer, 0));
	assert_int_equal(1, cbor_encode_indef_bytestring_start(buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]) {0x5F}), 1);
}

int main(void)
{
	const UnitTest tests[] = {
		unit_test(test_embedded_bytestring_start),
		unit_test(test_bytestring_start),
		unit_test(test_indef_bytestring_start)
	};
	return run_tests(tests);
}
