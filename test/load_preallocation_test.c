/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

/* cbor_load preallocates definite arrays and maps only as far as the input can
 * hold their items */

#include <stdlib.h>
#include <string.h>

#include "assertions.h"
#include "cbor.h"
#include "cbor/internal/builder_callbacks.h"
#include "cbor/internal/stack.h"
#include "test_allocator.h"

/* Smallest declared size that is checked against the input */
#define CHECKED_SIZE ((size_t)CBOR_MAX_UNCHECKED_DECLARED_SIZE + 1)
/* Tests decoding about CHECKED_SIZE items skip larger limits, which take
 * gigabytes */
#define MAX_DECODED_LIMIT 0x100000

static size_t largest_request, total_requested, reallocs;

static void* tracking_malloc(size_t size) {
  if (size > largest_request) largest_request = size;
  total_requested += size;
  return malloc(size);
}

static void* tracking_realloc(void* ptr, size_t size) {
  if (size > largest_request) largest_request = size;
  total_requested += size;
  reallocs++;
  return realloc(ptr, size);
}

/* Expect a truncation error, recording the allocations */
static void assert_truncated(const unsigned char* data, size_t size) {
  struct cbor_load_result res;
  largest_request = total_requested = 0;
  cbor_set_allocs(tracking_malloc, tracking_realloc, free);
  cbor_item_t* item = cbor_load(data, size, &res);
  cbor_set_allocs(malloc, realloc, free);
  assert_null(item);
  assert_true(res.error.code == CBOR_ERR_NOTENOUGHDATA);
  assert_size_equal(res.error.position, size);
}

static void assert_load_error(const unsigned char* data, size_t size,
                              cbor_error_code code, size_t position) {
  struct cbor_load_result res;
  cbor_item_t* item = cbor_load(data, size, &res);
  assert_null(item);
  assert_true(res.error.code == code);
  assert_size_equal(res.error.position, position);
}

/* [[[[0, {0: 0}]]]] has no spare byte */
unsigned char tight[] = {0x81, 0x81, 0x81, 0x82, 0x00, 0xA1, 0x00, 0x00};

static void test_tight_input(void** _state _CBOR_UNUSED) {
  struct cbor_load_result res;
  cbor_item_t* item = cbor_load(tight, sizeof(tight), &res);
  assert_non_null(item);
  assert_size_equal(res.read, sizeof(tight));
  cbor_item_t* level = item;
  for (int i = 0; i < 3; i++) {
    assert_size_equal(cbor_array_size(level), 1);
    assert_size_equal(cbor_array_allocated(level), 1);
    level = cbor_array_handle(level)[0];
  }
  assert_size_equal(cbor_array_size(level), 2);
  assert_size_equal(cbor_array_allocated(level), 2);
  assert_size_equal(cbor_map_size(cbor_array_handle(level)[1]), 1);
  assert_size_equal(cbor_map_allocated(cbor_array_handle(level)[1]), 1);
  cbor_decref(&item);

  assert_load_error(tight, sizeof(tight) - 1, CBOR_ERR_NOTENOUGHDATA,
                    sizeof(tight) - 1);
}

/* [{X: 0, 0: 0, ...}, 0, ...] or [{0: X, 0: 0, ...}, 0, ...], where the outer
 * array, the outer map and X, an array or a map of zeros, declare CHECKED_SIZE
 * items or pairs. After X's items, the input holds only a byte for each item
 * the outer containers still expect. */
static unsigned char* tight_nested(bool map, bool value, size_t* size) {
  unsigned char* data = malloc(3 * 9 + 5 * CHECKED_SIZE);
  *size = cbor_encode_array_start(CHECKED_SIZE, data, 9);
  *size += cbor_encode_map_start(CHECKED_SIZE, data + *size, 9);
  if (value) data[(*size)++] = 0x00;
  *size += map ? cbor_encode_map_start(CHECKED_SIZE, data + *size, 9)
               : cbor_encode_array_start(CHECKED_SIZE, data + *size, 9);
  size_t rest =
      (map ? 2 : 1) * CHECKED_SIZE + (value ? 0 : 1) + 3 * (CHECKED_SIZE - 1);
  memset(data + *size, 0x00, rest);
  *size += rest;
  return data;
}

/* Valid input is preallocated as declared, so its storage never grows */
static void test_tight_nested_input(void** _state _CBOR_UNUSED) {
  if (CBOR_MAX_UNCHECKED_DECLARED_SIZE > MAX_DECODED_LIMIT) skip();
  for (int map = 0; map <= 1; map++) {
    for (int value = 0; value <= 1; value++) {
      size_t size;
      unsigned char* data = tight_nested(map, value, &size);
      struct cbor_load_result res;
      reallocs = 0;
      cbor_set_allocs(tracking_malloc, tracking_realloc, free);
      cbor_item_t* item = cbor_load(data, size, &res);
      cbor_set_allocs(malloc, realloc, free);
      assert_non_null(item);
      assert_size_equal(res.read, size);
      assert_size_equal(reallocs, 0);
      struct cbor_pair pair = cbor_map_handle(cbor_array_handle(item)[0])[0];
      cbor_item_t* inner = value ? pair.value : pair.key;
      assert_size_equal(map ? cbor_map_size(inner) : cbor_array_size(inner),
                        CHECKED_SIZE);
      cbor_decref(&item);
      free(data);
    }
  }
}

static void test_large_valid_containers(void** _state _CBOR_UNUSED) {
  /* [0, 0, ..., 0] with 1000 items, then {0: 0, ...} with 1000 pairs */
  unsigned char data[1 + 3 + 1000 + 3 + 2000];
  size_t pos = 0;
  data[pos++] = 0x82;
  data[pos++] = 0x99;
  data[pos++] = 0x03;
  data[pos++] = 0xE8;
  memset(data + pos, 0, 1000);
  pos += 1000;
  data[pos++] = 0xB9;
  data[pos++] = 0x03;
  data[pos++] = 0xE8;
  memset(data + pos, 0, 2000);
  pos += 2000;
  assert_size_equal(pos, sizeof(data));

  struct cbor_load_result res;
  cbor_item_t* item = cbor_load(data, sizeof(data), &res);
  assert_non_null(item);
  cbor_item_t* array = cbor_array_handle(item)[0];
  assert_size_equal(cbor_array_size(array), 1000);
  assert_size_equal(cbor_array_allocated(array), 1000);
  cbor_item_t* map = cbor_array_handle(item)[1];
  assert_size_equal(cbor_map_size(map), 1000);
  assert_size_equal(cbor_map_allocated(map), 1000);
  cbor_decref(&item);
}

/* 5-byte headers declaring 2^28 items or pairs */
unsigned char huge_array[] = {0x9A, 0x10, 0x00, 0x00, 0x00};
unsigned char huge_map[] = {0xBA, 0x10, 0x00, 0x00, 0x00};
/* Nested in containers that still expect items */
unsigned char huge_nested[] = {0x84, 0xA2, 0x00, 0xD8, 0x20, 0x9F,
                               0x9A, 0x10, 0x00, 0x00, 0x00};

static void test_oversized_claims(void** _state _CBOR_UNUSED) {
  if (CBOR_MAX_UNCHECKED_DECLARED_SIZE >= 0x10000000) skip();
  assert_truncated(huge_array, sizeof(huge_array));
  assert_true(total_requested < 256);
  assert_truncated(huge_map, sizeof(huge_map));
  assert_true(total_requested < 256);
  assert_truncated(huge_nested, sizeof(huge_nested));
  assert_true(total_requested < 1024);
}

#ifdef EIGHT_BYTE_SIZE_T
/* A 9-byte header declaring 2^33 items, 64 GiB of storage */
unsigned char huge_array_64[] = {0x9B, 0x00, 0x00, 0x00, 0x02,
                                 0x00, 0x00, 0x00, 0x00};

static void test_oversized_claims_64(void** _state _CBOR_UNUSED) {
  if (CBOR_MAX_UNCHECKED_DECLARED_SIZE >= 0x200000000) skip();
  assert_truncated(huge_array_64, sizeof(huge_array_64));
  assert_true(total_requested < 256);
}
#endif

/* Declarations of more items than size_t can count, with two per map pair,
 * fail after the header */
static void test_uncountable_claims(void** _state _CBOR_UNUSED) {
  /* 2^63 pairs */
  const unsigned char map[] = {0xBB, 0x80, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00};
  assert_load_error(map, sizeof(map), CBOR_ERR_MEMERROR, 9);
#ifndef EIGHT_BYTE_SIZE_T
  /* 2^32 items, then 2^31 pairs */
  const unsigned char array_32[] = {0x9B, 0x00, 0x00, 0x00, 0x01,
                                    0x00, 0x00, 0x00, 0x00, 0x00};
  assert_load_error(array_32, sizeof(array_32), CBOR_ERR_MEMERROR, 9);
  const unsigned char map_32[] = {0xBA, 0x80, 0x00, 0x00, 0x00, 0x00};
  assert_load_error(map_32, sizeof(map_32), CBOR_ERR_MEMERROR, 5);
#endif
}

/* Each header declares 2^20 items and is the first item of the one before */
#define CHAIN_LENGTH 400

static void test_chained_claims(void** _state _CBOR_UNUSED) {
  if (CBOR_MAX_UNCHECKED_DECLARED_SIZE >= 0x100000) skip();
  const unsigned char array_header[] = {0x9A, 0x00, 0x10, 0x00, 0x00};
  const unsigned char map_header[] = {0xBA, 0x00, 0x10, 0x00, 0x00};
  /* With an indefinite array and a tag in between */
  const unsigned char wrapped_header[] = {0x9A, 0x00, 0x10, 0x00,
                                          0x00, 0x9F, 0xC6};
  const unsigned char* headers[] = {array_header, map_header, wrapped_header};
  size_t header_sizes[] = {sizeof(array_header), sizeof(map_header),
                           sizeof(wrapped_header)};
  for (size_t i = 0; i < 3; i++) {
    size_t size = CHAIN_LENGTH * header_sizes[i];
    unsigned char* data = malloc(size);
    for (size_t j = 0; j < CHAIN_LENGTH; j++) {
      memcpy(data + j * header_sizes[i], headers[i], header_sizes[i]);
    }
    assert_truncated(data, size);
    /* Only the first container gets storage, for what the input can hold */
    assert_true(largest_request <= size * sizeof(cbor_item_t*));
    assert_true(total_requested < 64 * size);
    free(data);
  }
}

/* An array of 32 items whose first one is an array or map declaring
 * CHECKED_SIZE items, then `items` bytes of 0x01. The inner container cannot
 * hold all its items if the outer one is to get the rest, so its storage starts
 * short. */
static unsigned char* grown(bool map, size_t items, size_t* size) {
  unsigned char* data = malloc(2 + 9 + items + 1);
  data[0] = 0x98;
  data[1] = 0x20;
  *size = 2 + (map ? cbor_encode_map_start(CHECKED_SIZE, data + 2, 9)
                   : cbor_encode_array_start(CHECKED_SIZE, data + 2, 9));
  memset(data + *size, 0x01, items);
  *size += items;
  return data;
}

/* Decoding fails where it would with the declared size preallocated */
static void test_same_errors(void** _state _CBOR_UNUSED) {
  if (CBOR_MAX_UNCHECKED_DECLARED_SIZE > MAX_DECODED_LIMIT) skip();
  /* Array of 100 items, then a break or a reserved value */
  const unsigned char break_after_claim[] = {0x98, 0x64, 0xFF};
  assert_load_error(break_after_claim, sizeof(break_after_claim),
                    CBOR_ERR_SYNTAXERROR, 3);
  const unsigned char reserved_after_claim[] = {0x98, 0x64, 0x1C};
  assert_load_error(reserved_after_claim, sizeof(reserved_after_claim),
                    CBOR_ERR_MALFORMATED, 2);
  const unsigned char break_after_huge_claim[] = {0x9A, 0x11, 0x27,
                                                  0x00, 0x00, 0xFF};
  assert_load_error(break_after_huge_claim, sizeof(break_after_huge_claim),
                    CBOR_ERR_SYNTAXERROR, 6);
  const unsigned char reserved_after_huge_claim[] = {0x9A, 0x11, 0x27,
                                                     0x00, 0x00, 0x1C};
  assert_load_error(reserved_after_huge_claim,
                    sizeof(reserved_after_huge_claim), CBOR_ERR_MALFORMATED, 5);

  /* The inner container grows to completion, then a break follows */
  size_t size;
  unsigned char* data = grown(false, CHECKED_SIZE, &size);
  assert_truncated(data, size);
  data[size] = 0xFF;
  assert_load_error(data, size + 1, CBOR_ERR_SYNTAXERROR, size + 1);
  free(data);
  data = grown(true, 2 * CHECKED_SIZE, &size);
  assert_truncated(data, size);
  data[size] = 0xFF;
  assert_load_error(data, size + 1, CBOR_ERR_SYNTAXERROR, size + 1);
  free(data);
}

static void test_growth_failure(void** _state _CBOR_UNUSED) {
  struct cbor_load_result res;
  size_t size;
  for (int map = 0; map <= 1; map++) {
    unsigned char* data = grown(map, 1, &size);
    WITH_MOCK_MALLOC(
        {
          assert_null(cbor_load(data, size, &res));
          assert_true(res.error.code == CBOR_ERR_MEMERROR);
          assert_size_equal(res.error.position, size);
        },
        // Outer array (item, storage, stack record), inner container (item,
        // stack record; no storage), the first item, and growing for it
        7, MALLOC, MALLOC, MALLOC, MALLOC, MALLOC, MALLOC, REALLOC_FAIL);
    free(data);
  }
}

/* Up to CBOR_MAX_UNCHECKED_DECLARED_SIZE, the declared size is preallocated.
 * Each map is the first key of the one before. */
static void test_unchecked_claims(void** _state _CBOR_UNUSED) {
  if (CBOR_MAX_UNCHECKED_DECLARED_SIZE == 0 ||
      CBOR_MAX_UNCHECKED_DECLARED_SIZE > 4096)
    skip();
  unsigned char header[9];
  size_t header_size =
      cbor_encode_map_start(CBOR_MAX_UNCHECKED_DECLARED_SIZE, header, 9);
  size_t size = 100 * header_size;
  unsigned char* data = malloc(size);
  for (size_t i = 0; i < 100; i++)
    memcpy(data + i * header_size, header, header_size);
  assert_truncated(data, size);
  assert_true(total_requested <=
              100 *
                  (CBOR_MAX_UNCHECKED_DECLARED_SIZE * sizeof(struct cbor_pair) +
                   sizeof(cbor_item_t) + sizeof(struct _cbor_stack_record)));
  free(data);
}

/* Without the input size, the builder preallocates the declared size */
static void test_unknown_input_size(void** _state _CBOR_UNUSED) {
  struct _cbor_stack stack = _cbor_stack_init();
  struct _cbor_decoder_context context = {.stack = &stack};

  cbor_builder_array_start_callback(&context, 1000);
  assert_false(context.creation_failed);
  assert_size_equal(cbor_array_allocated(stack.top->item), 1000);
  cbor_decref(&stack.top->item);
  _cbor_stack_pop(&stack);

  cbor_builder_map_start_callback(&context, 1000);
  assert_false(context.creation_failed);
  assert_size_equal(cbor_map_allocated(stack.top->item), 1000);
  cbor_decref(&stack.top->item);
  _cbor_stack_pop(&stack);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_tight_input),
      cmocka_unit_test(test_tight_nested_input),
      cmocka_unit_test(test_large_valid_containers),
      cmocka_unit_test(test_oversized_claims),
#ifdef EIGHT_BYTE_SIZE_T
      cmocka_unit_test(test_oversized_claims_64),
#endif
      cmocka_unit_test(test_uncountable_claims),
      cmocka_unit_test(test_chained_claims),
      cmocka_unit_test(test_same_errors),
      cmocka_unit_test(test_growth_failure),
      cmocka_unit_test(test_unchecked_claims),
      cmocka_unit_test(test_unknown_input_size),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
