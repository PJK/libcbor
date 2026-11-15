/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#include "assertions.h"
#include "cbor.h"

/* These tests verify behavior on interesting randomly generated inputs from the
 * fuzzer */

cbor_item_t* item;
struct cbor_load_result res;

/* Map start + array with embedded length */
unsigned char data1[] = {0xA9, 0x85};
static void test_1(void** _state _CBOR_UNUSED) {
  item = cbor_load(data1, 2, &res);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
  assert_size_equal(res.error.position, 2);
}

unsigned char data2[] = {0x9D};
static void test_2(void** _state _CBOR_UNUSED) {
  item = cbor_load(data2, 1, &res);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_MALFORMATED);
  assert_size_equal(res.error.position, 0);
}

unsigned char data3[] = {0xD6};
static void test_3(void** _state _CBOR_UNUSED) {
  item = cbor_load(data3, 1, &res);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
  assert_size_equal(res.error.position, 1);
}

/* Map declaring ~3.2 * 10^9 pairs */
unsigned char data4[] = {0xBA, 0xC1, 0xE8, 0x3E, 0xE7, 0x20, 0xA8};
static void test_4(void** _state _CBOR_UNUSED) {
  item = cbor_load(data4, 7, &res);
  assert_null(item);
#ifdef EIGHT_BYTE_SIZE_T
  /* Only the pairs that fit in the input are preallocated */
  assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
  assert_size_equal(res.error.position, 7);
#else
  /* Twice the pair count, the number of items, does not fit in size_t */
  assert_true(res.error.code == CBOR_ERR_MEMERROR);
  assert_size_equal(res.error.position, 5);
#endif
}

/* Array declaring ~3.7 * 10^9 items */
unsigned char data5[] = {0x9A, 0xDA, 0x3A, 0xB2, 0x7F, 0x29};
static void test_5(void** _state _CBOR_UNUSED) {
  item = cbor_load(data5, 6, &res);
  assert_null(item);
  /* Only the items that fit in the input are preallocated */
  assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
  assert_size_equal(res.error.position, 6);
}

unsigned char data6[] = {0x7F, 0x21, 0x4C, 0x02, 0x40};
static void test_6(void** _state _CBOR_UNUSED) {
  item = cbor_load(data6, 5, &res);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_SYNTAXERROR);
  assert_size_equal(res.error.position, 2);
}

#ifdef EIGHT_BYTE_SIZE_T
/* Extremely high size value (its storage size overflows size_t). Only the
 * items that fit in the input are preallocated. Only works with 64b sizes */
unsigned char data7[] = {0xA2, 0x9B, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static void test_7(void** _state _CBOR_UNUSED) {
  item = cbor_load(data7, 16, &res);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
  assert_size_equal(res.error.position, 16);
}
#endif

unsigned char data8[] = {0xA3, 0x64, 0x68, 0x61, 0x6C, 0x66, 0xFF, 0x00,
                         0x00, 0x66, 0x73, 0x69, 0x6E, 0x67, 0x6C, 0x65,
                         0xFA, 0x7F, 0x7F, 0xFF, 0xFF, 0x6D, 0x73, 0x69,
                         0x6D, 0x70, 0x6C, 0x65, 0x20, 0x76, 0x61, 0x6C,
                         0x75, 0x65, 0x73, 0x83, 0xF5, 0xF4, 0xF6};
static void test_8(void** _state _CBOR_UNUSED) {
  item = cbor_load(data8, 39, &res);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_SYNTAXERROR);
  assert_size_equal(res.error.position, 7);
}

unsigned char data9[] = {0xBF, 0x05, 0xFF, 0x00, 0x00, 0x00, 0x10, 0x04};
static void test_9(void** _state _CBOR_UNUSED) {
  item = cbor_load(data9, 8, &res);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_SYNTAXERROR);
  assert_size_equal(res.error.position, 3);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_1), cmocka_unit_test(test_2),
      cmocka_unit_test(test_3), cmocka_unit_test(test_4),
      cmocka_unit_test(test_5), cmocka_unit_test(test_6),
#ifdef EIGHT_BYTE_SIZE_T
      cmocka_unit_test(test_7),
#endif
      cmocka_unit_test(test_8), cmocka_unit_test(test_9),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
