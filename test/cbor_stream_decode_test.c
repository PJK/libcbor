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
	assert_decoder_result(1, CBOR_DECODER_FINISHED,
		decode(embedded_uint8_data, 1)
	);

	assert_uint8_eq(1);
	assert_decoder_result(1, CBOR_DECODER_FINISHED,
		decode(embedded_uint8_data + 1, 1)
	);

	assert_uint8_eq(5);
	assert_decoder_result(1, CBOR_DECODER_FINISHED,
		decode(embedded_uint8_data + 2, 1)
	);

	assert_uint8_eq(23);
	assert_decoder_result(1, CBOR_DECODER_FINISHED,
		decode(embedded_uint8_data + 3, 1)
	);
}


unsigned char uint8_data[] = { 0x18, 0x83, 0x18, 0xFF };
static void test_uint8_decoding(void **state)
{
	assert_uint8_eq(0x83);
	assert_decoder_result(2, CBOR_DECODER_FINISHED,
		decode(uint8_data, 2)
	);

	assert_uint8_eq(0xFF);
	assert_decoder_result(2, CBOR_DECODER_FINISHED,
		decode(uint8_data + 2, 2)
	);
}

unsigned char uint16_data[] = { 0x19, 0x01, 0xf4 };
static void test_uint16_decoding(void **state)
{

	assert_uint16_eq(500);
	assert_decoder_result(3, CBOR_DECODER_FINISHED,
		decode(uint16_data, 3)
	);
}

unsigned char uint32_data[] = { 0x1a, 0xa5, 0xf7, 0x02, 0xb3 };
static void test_uint32_decoding(void **state)
{
	assert_uint32_eq((uint32_t) 2784428723UL);
	assert_decoder_result(5, CBOR_DECODER_FINISHED,
		decode(uint32_data, 5)
	);
}

unsigned char uint64_data[] = { 0x1b, 0xa5, 0xf7, 0x02, 0xb3, 0xa5, 0xf7, 0x02, 0xb3 };
static void test_uint64_decoding(void **state)
{
	assert_uint64_eq(11959030306112471731ULL);
	assert_decoder_result(9, CBOR_DECODER_FINISHED,
		decode(uint64_data, 9)
	);
}

int main(void)
{
	set_decoder(&cbor_stream_decode);
	const UnitTest tests[] = {
		unit_test(test_uint8_embedded_decoding),
		unit_test(test_uint8_decoding),
		unit_test(test_uint16_decoding),
		unit_test(test_uint32_decoding),
		unit_test(test_uint64_decoding)
	};
	return run_tests(tests);
}
