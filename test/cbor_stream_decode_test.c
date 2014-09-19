#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "cbor.h"
#include <inttypes.h>
#include "stream_expectations.h"
#include "assertions.h"

unsigned char embedded_uint8_data[] = { 0x00, 0x01, 0x05, 0x17 };

static void test_uint8_embedded_decoding(void **state)
{
	assert_uint8_eq(0);
	decode(embedded_uint8_data, 1);
	assert_uint8_eq(1);
	decode(embedded_uint8_data + 1, 1);
	assert_uint8_eq(5);
	decode(embedded_uint8_data + 2, 1);
	assert_uint8_eq(23);
	decode(embedded_uint8_data + 3, 1);
}


unsigned char uint8_data[] = { 0x18, 0x83, 0x18, 0xFF };
static void test_uint8_decoding(void **state)
{
	assert_uint8_eq(0x83);
	assert_decoder_result(2, CBOR_DECODER_FINISHED,
		decode(uint8_data, 2)
	);
	assert_uint8_eq(0xFF);
	decode(uint8_data + 2, 2);
}

int main(void)
{
	set_decoder(&cbor_stream_decode);
	const UnitTest tests[] = {
		unit_test(test_uint8_embedded_decoding),
		unit_test(test_uint8_decoding)
	};
	return run_tests(tests);
}
