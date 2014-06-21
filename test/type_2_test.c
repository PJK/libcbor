#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>


cbor_item_t * bs;
struct cbor_load_result res;

unsigned char data1[] = { 0x40, 0xFF };
unsigned char data2[] = { 0x40, 0xA1 };

static void test_empty_bs(void **state) {
	bs = cbor_load(data1, 1, 0, &res);
	assert_non_null(bs);
	assert_true(cbor_typeof(bs) == CBOR_TYPE_BYTESTRING);
	assert_true(cbor_isa_bytestring(bs));
	assert_true(cbor_bytestring_length(bs) == 0);
}

int main(void) {
	const UnitTest tests[] = {
		unit_test(test_empty_bs)
	};
	return run_tests(tests);
}
