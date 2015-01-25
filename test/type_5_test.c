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


unsigned char simple_map[] = { 0xA2, 0x01, 0x02, 0x03, 0x04 }; /* {1: 2, 3: 4} */
static void test_simple_map(void **state)
{
	map = cbor_load(simple_map, 5, CBOR_FLAGS_NONE, &res);
	assert_non_null(map);
	assert_true(cbor_typeof(map) == CBOR_TYPE_MAP);
	assert_true(cbor_isa_map(map));
	assert_true(cbor_map_is_definite(map));
	assert_true(cbor_map_size(map) == 2);
	assert_true(res.read == 5);
	struct cbor_pair * handle = cbor_map_handle(map);
	assert_uint8(handle[0].key, 1);
	assert_uint8(handle[0].value, 2);
	assert_uint8(handle[1].key, 3);
	assert_uint8(handle[1].value, 4);
	cbor_decref(&map);
	assert_null(map);
}

unsigned char simple_indef_map[] = { 0xBF, 0x01, 0x02, 0x03, 0x04, 0xFF }; /* {_ 1: 2, 3: 4} */
static void test_indef_simple_map(void **state)
{
	map = cbor_load(simple_indef_map, 6, CBOR_FLAGS_NONE, &res);
	assert_non_null(map);
	assert_true(cbor_typeof(map) == CBOR_TYPE_MAP);
	assert_true(cbor_isa_map(map));
	assert_true(cbor_map_is_indefinite(map));
	assert_true(cbor_map_size(map) == 2);
	assert_true(res.read == 6);
	struct cbor_pair * handle = cbor_map_handle(map);
	assert_uint8(handle[0].key, 1);
	assert_uint8(handle[0].value, 2);
	assert_uint8(handle[1].key, 3);
	assert_uint8(handle[1].value, 4);
	cbor_decref(&map);
	assert_null(map);
}

int main(void) {
	const UnitTest tests[] = {
		unit_test(test_empty_map),
		unit_test(test_simple_map),
		unit_test(test_indef_simple_map)
	};
	return run_tests(tests);
}
