/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#include "assertions.h"
#include <stdlib.h>

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

#ifdef SANE_MALLOC
unsigned char data4[] = {0xBA, 0xC1, 0xE8, 0x3E, 0xE7, 0x20, 0xA8};
static void test_4(void** _state _CBOR_UNUSED) {
  item = cbor_load(data4, 7, &res);
  assert_null(item);
  /* The claimed 3.2e9 pairs are not preallocated, so decoding runs until the
   * data does */
  assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
  assert_size_equal(res.error.position, 7);
}

unsigned char data5[] = {0x9A, 0xDA, 0x3A, 0xB2, 0x7F, 0x29};
static void test_5(void** _state _CBOR_UNUSED) {
  item = cbor_load(data5, 6, &res);
  assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
  assert_null(item);
  assert_size_equal(res.error.position, 6);
  /* Indef string expectation mismatch */
}
#endif

unsigned char data6[] = {0x7F, 0x21, 0x4C, 0x02, 0x40};
static void test_6(void** _state _CBOR_UNUSED) {
  item = cbor_load(data6, 5, &res);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_SYNTAXERROR);
  assert_size_equal(res.error.position, 2);
}

#ifdef EIGHT_BYTE_SIZE_T
/* Extremely high size value (overflows size_t in representation size). Only
 * works with 64b sizes */
unsigned char data7[] = {0xA2, 0x9B, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static void test_7(void** _state _CBOR_UNUSED) {
  item = cbor_load(data7, 16, &res);
  assert_null(item);
  /* The array is not preallocated, so decoding runs until the data does */
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

/* Definite headers claiming more items than the input holds get storage only
 * for what the input can hold (#418). */
static size_t claim_allocated;
static void* claim_malloc(size_t size) {
  if (size > claim_allocated) claim_allocated = size;
  return malloc(size);
}
static void* claim_realloc(void* ptr, size_t size) {
  if (size > claim_allocated) claim_allocated = size;
  return realloc(ptr, size);
}

static void assert_claim_bounded(const unsigned char* data, size_t len,
                                 size_t position) {
  claim_allocated = 0;
  cbor_set_allocs(claim_malloc, claim_realloc, free);
  item = cbor_load(data, len, &res);
  cbor_set_allocs(malloc, realloc, free);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
  assert_size_equal(res.error.position, position);
  assert_true(claim_allocated < 1024);
}

unsigned char claim_array[] = {0x9A, 0x11, 0x27, 0x00, 0x00};
unsigned char claim_map[] = {0xBA, 0x10, 0x00, 0x00, 0x00};
/* An oversized claim inside open containers */
unsigned char claim_nested[] = {0x84, 0x83, 0x82, 0x9A, 0x01, 0x00, 0x00, 0x00};
/* Many oversized claims, each nested in the previous one */
unsigned char claim_chain[] = {0x9A, 0x01, 0x00, 0x00, 0x00, 0x9A, 0x01, 0x00,
                               0x00, 0x00, 0x9A, 0x01, 0x00, 0x00, 0x00, 0x9A,
                               0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static void test_claims(void** _state _CBOR_UNUSED) {
  assert_claim_bounded(claim_array, sizeof(claim_array), 5);
  assert_claim_bounded(claim_map, sizeof(claim_map), 5);
  assert_claim_bounded(claim_nested, sizeof(claim_nested), 8);
  assert_claim_bounded(claim_chain, sizeof(claim_chain), sizeof(claim_chain));
}

/* Exactly enough bytes, deeply nested: must still decode */
unsigned char claim_tight[] = {0x81, 0x81, 0x81, 0x82, 0x00, 0xA1, 0x00, 0x00};
static void test_claims_tight(void** _state _CBOR_UNUSED) {
  item = cbor_load(claim_tight, sizeof(claim_tight), &res);
  assert_non_null(item);
  assert_size_equal(res.read, sizeof(claim_tight));
  cbor_decref(&item);
  /* One byte short is a plain truncation */
  item = cbor_load(claim_tight, sizeof(claim_tight) - 1, &res);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
  assert_size_equal(res.error.position, sizeof(claim_tight) - 1);
}

/* The inner container claims more than is left once the outer one's second
 * item is set aside, so it starts short, grows, and is completed; the input
 * then ends before the outer container does. */
unsigned char claim_grow_array[] = {0x82, 0x83, 0x01, 0x02, 0x03};
unsigned char claim_grow_map[] = {0x82, 0xA2, 0x01, 0x02, 0x03, 0x04};
/* Same shape with the outer item present: decodes */
unsigned char claim_grow_done[] = {0x82, 0x83, 0x01, 0x02, 0x03,
                                   0xA2, 0x01, 0x02, 0x03, 0x04};
static void test_claims_grow(void** _state _CBOR_UNUSED) {
  item = cbor_load(claim_grow_array, sizeof(claim_grow_array), &res);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
  assert_size_equal(res.error.position, sizeof(claim_grow_array));
  item = cbor_load(claim_grow_map, sizeof(claim_grow_map), &res);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
  assert_size_equal(res.error.position, sizeof(claim_grow_map));
  item = cbor_load(claim_grow_done, sizeof(claim_grow_done), &res);
  assert_non_null(item);
  assert_size_equal(cbor_array_size(item), 2);
  assert_size_equal(cbor_array_size(cbor_array_handle(item)[0]), 3);
  assert_size_equal(cbor_array_allocated(cbor_array_handle(item)[0]), 3);
  assert_size_equal(cbor_map_size(cbor_array_handle(item)[1]), 2);
  cbor_decref(&item);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_1), cmocka_unit_test(test_2),
      cmocka_unit_test(test_3),
#ifdef SANE_MALLOC
      cmocka_unit_test(test_4), cmocka_unit_test(test_5),
#endif
      cmocka_unit_test(test_6),
#ifdef EIGHT_BYTE_SIZE_T
      cmocka_unit_test(test_7),
#endif
      cmocka_unit_test(test_8), cmocka_unit_test(test_9),
      cmocka_unit_test(test_claims), cmocka_unit_test(test_claims_tight),
      cmocka_unit_test(test_claims_grow),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
