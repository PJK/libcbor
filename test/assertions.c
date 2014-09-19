#include "assertions.h"

void assert_uint8(cbor_item_t * item, uint8_t num)
{
	assert_true(cbor_isa_uint(item));
	assert_true(cbor_get_uint8(item) == num);
}

void assert_decoder_result(size_t read, enum cbor_decoder_status status, struct cbor_decoder_result result)
{
	assert_true(read == result.read);
	assert_true(status == result.status);
}
