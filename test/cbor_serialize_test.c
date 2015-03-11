#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include "../src/cbor.h"
#include <inttypes.h>


unsigned char buffer[512];

static void test_serialize_uint8(void **state) {
	cbor_item_t * item = cbor_new_int8();
	cbor_set_uint8(item, 0);
	assert_int_equal(1, cbor_serialize(item, buffer, 512));
	assert_memory_equal(buffer, (unsigned char[]){ 0x00 }, 1);
}

static void test_serialize_uint16(void **state) {
	cbor_item_t * item = cbor_new_int16();
	cbor_set_uint16(item, 1000);
	assert_int_equal(3, cbor_serialize(item, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0x19, 0x03, 0xE8 }), 3);
}

static void test_serialize_uint32(void **state) {
	cbor_item_t * item = cbor_new_int32();
	cbor_set_uint32(item, 1000000);
	assert_int_equal(5, cbor_serialize(item, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0x1A, 0x00, 0x0F, 0x42, 0x40 }), 5);
}

static void test_serialize_uint64(void **state) {
	cbor_item_t * item = cbor_new_int64();
	cbor_set_uint64(item, 1000000000000);
	assert_int_equal(9, cbor_serialize(item, buffer, 512));
	assert_memory_equal(buffer, ((unsigned char[]){ 0x1B, 0x00, 0x00, 0x00, 0xE8, 0xD4, 0xA5, 0x10, 0x00 }), 9);
}

int main(void) {
	const UnitTest tests[] = {
		unit_test(test_serialize_uint8),
		unit_test(test_serialize_uint16),
		unit_test(test_serialize_uint32),
		unit_test(test_serialize_uint64)
	};
	return run_tests(tests);
}
