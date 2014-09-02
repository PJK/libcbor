#include "assertions.h"

void assert_uint8(cbor_item_t * item, uint8_t num)
{
	assert_true(cbor_isa_uint(item));
	assert_true(cbor_get_uint8(item) == num);
}
