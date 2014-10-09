#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>
#include "assertions.h"


cbor_item_t * map;
struct cbor_load_result res;

unsigned char empty_map[] = { 0xA0 };
static void test_empty_map(void **state)
{
	map = cbor_load(empty_map, 1, CBOR_FLAGS_NONE, &res);
	assert_non_null(map);
	assert_true(cbor_typeof(map) == CBOR_TYPE_MAP);
	assert_true(cbor_isa_map(map));
	assert_true(cbor_map_size(map) == 0);
	assert_true(res.read == 1);
	cbor_decref(&map);
	assert_null(map);
}


int main(void) {
	const UnitTest tests[] = {
		unit_test(test_empty_map),
	};
	return run_tests(tests);
}
