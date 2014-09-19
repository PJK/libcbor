#include "stream_expectations.h"

void set_decoder(decoder_t * dec)
{
	decoder = dec;
}

/* Assertions builders */

void assert_uint8_eq(uint8_t actual)
{
	assertions_queue[queue_size++] = (struct test_assertion) { UINT8_EQ, (union test_expectation_data) { .int8 = actual } };
}

void assert_uint16_eq(uint16_t actual);
void assert_uint32_eq(uint32_t actual);
void assert_uint64_eq(uint64_t actual);

void assert_negint8_eq(uint8_t actual);
void assert_negint16_eq(uint16_t actual);
void assert_negint32_eq(uint32_t actual);
void assert_negint64_eq(uint64_t actual);


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

const struct cbor_callbacks asserting_callbacks = {
	.uint8 = &uint8_callback
};

struct cbor_decoder_result decode(cbor_data source, size_t source_size)
{
	struct cbor_decoder_result result = decoder(source, source_size, &asserting_callbacks);
	/* Check remaining assertions */
	assert_true(current_expectation == queue_size);
	/* Clean up */
	current_expectation = queue_size = 0;
	return result;
}


