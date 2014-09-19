#include "stream_expectations.h"

void set_decoder(decoder_t * dec)
{
	decoder = dec;
}

struct cbor_decoder_result decode(cbor_data source, size_t source_size, const struct cbor_callbacks * callbacks)
{
	struct cbor_decoder_result result = decoder(source, source_size, callbacks);
	/* Check remaining assertions */

	return result;
}

/* Assertions builders */

void assert_uint8_eq(uint8_t actual, union test_expectation_data data);
void assert_uint16_eq(uint16_t actual, union test_expectation_data data);
void assert_uint32_eq(uint32_t actual, union test_expectation_data data);
void assert_uint64_eq(uint64_t actual, union test_expectation_data data);

void assert_negint8_eq(uint8_t actual, union test_expectation_data data);
void assert_negint16_eq(uint16_t actual, union test_expectation_data data);
void assert_negint32_eq(uint32_t actual, union test_expectation_data data);
void assert_negint64_eq(uint64_t actual, union test_expectation_data data);


/* Callbacks */
union test_expectation_data current()
{
	return assertions_queue[current_expectation++].data;
}

enum cbor_callback_result uint8_callback(uint8_t actual)
{
	assert_true(current().int8 == actual);
	return CBOR_CALLBACK_OK;
}
