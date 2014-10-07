#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include "assertions.h"
#include <inttypes.h>

cbor_item_t * float_ctrl;
struct cbor_load_result res;


unsigned char float2_data[] = { 0xF9, 0x7B, 0xFF };
static void test_float2(void **state) {
	float_ctrl = cbor_load(float2_data, 3, CBOR_FLAGS_NONE, &res);
	assert_true(cbor_isa_float_ctrl(float_ctrl));
	assert_true(cbor_is_float(float_ctrl));
	assert_true(cbor_float_get_width(float_ctrl) == CBOR_FLOAT_16);
	assert_true(cbor_float_get_float2(float_ctrl) == 65504.0);
	cbor_decref(&float_ctrl);
	assert_null(float_ctrl);
}



int main(void) {
	const UnitTest tests[] = {
		unit_test(test_float2)
	};
	return run_tests(tests);
}
