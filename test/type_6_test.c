#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>

cbor_item_t * tag;
struct cbor_load_result res;


unsigned char embedded_tag_data[] = { 0xC0, 0x00 }; /* Tag 0 + uint 0 C */
static void test_embedded_tag(void **state) {
	tag = cbor_load(embedded_tag_data, 2, CBOR_FLAGS_NONE, &res);
	assert_true(cbor_typeof(tag) == CBOR_TYPE_TAG);
	cbor_decref(&tag);
	assert_null(tag);
}


int main(void) {
	const UnitTest tests[] = {
		unit_test(test_embedded_tag)
	};
	return run_tests(tests);
}
