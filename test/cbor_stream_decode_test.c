#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>


int _assert_uint8_calls = 0;

int _assert_uint8_expected_calls = 0;
uint8_t * _assert_uint8_expectations = NULL;
enum cbor_callback_result _assert_uint8(uint8_t val)
{
	assert_int_equal(val, _assert_uint8_expectations[_assert_uint8_calls++]);
	return CBOR_CALLBACK_OK;
};

struct cbor_callbacks callbacks = { .uint8 = &_assert_uint8 };

void cleanup()
{
	callbacks.uint8 = NULL;
	assert_int_equal(_assert_uint8_calls, _assert_uint8_expected_calls);
	_assert_uint8_calls = 0;
	_assert_uint8_expected_calls = 0;
}

unsigned char data1[] = { 0x00, 0x01, 0x05, 0x17 };
uint8_t test_simple_int_load_expectations[] = { 0, 1, 5, 23 };

static void test_simple_int_load(void **state)
{
	_assert_uint8_expectations = test_simple_int_load_expectations;
	_assert_uint8_expected_calls = 4;
	cbor_stream_decode(data1, 4, &callbacks);
	cbor_stream_decode(data1 + 1, 3, &callbacks);
	cbor_stream_decode(data1 + 2, 2, &callbacks);
	cbor_stream_decode(data1 + 3, 1, &callbacks);
	cleanup();
}

int main(void)
{
	const UnitTest tests[] = {
		unit_test(test_simple_int_load)
	};
	return run_tests(tests);
}
