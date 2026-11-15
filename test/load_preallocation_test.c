/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

/* cbor_load preallocates definite arrays and maps only as far as the rest of
 * the input can hold their items */

#include <stdlib.h>
#include <string.h>

#include "assertions.h"
#include "cbor.h"
#include "cbor/internal/builder_callbacks.h"
#include "cbor/internal/stack.h"
#include "test_allocator.h"

static size_t largest_request, total_requested;

static void* tracking_malloc(size_t size) {
  if (size > largest_request) largest_request = size;
  total_requested += size;
  return malloc(size);
}

static void* tracking_realloc(void* ptr, size_t size) {
  if (size > largest_request) largest_request = size;
  total_requested += size;
  return realloc(ptr, size);
}

/* Load the input, recording the allocations it takes, and expect it to be
 * truncated */
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

/* [[[[0, {0: 0}]]]] has no spare byte for any declared item */
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
/* The same, nested in containers that still expect items */
unsigned char huge_nested[] = {0x84, 0xA2, 0x00, 0xD8, 0x20, 0x9F,
                               0x9A, 0x10, 0x00, 0x00, 0x00};

static void test_oversized_claims(void** _state _CBOR_UNUSED) {
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
  assert_truncated(huge_array_64, sizeof(huge_array_64));
  assert_true(total_requested < 256);
}
#endif

/* Each header declares 2^20 items and is the first item of the one before.
 * Preallocating each of them would take gigabytes. */
#define CHAIN_LENGTH 400

static void test_chained_claims(void** _state _CBOR_UNUSED) {
  const unsigned char array_header[] = {0x9A, 0x00, 0x10, 0x00, 0x00};
  const unsigned char map_header[] = {0xBA, 0x00, 0x10, 0x00, 0x00};
  /* Indefinite arrays and tags in between expect no further items */
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
    /* The first container takes the storage for all the items the input can
     * hold; the others take none until their items arrive */
    assert_true(largest_request <= size * sizeof(cbor_item_t*));
    assert_true(total_requested < 64 * size);
    free(data);
  }
}

/* Where the declared size cannot be preallocated, decoding fails exactly where
 * it would if it had been */
static void test_same_errors(void** _state _CBOR_UNUSED) {
  /* Array of 100 items, then a break or a reserved additional information
   * value */
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

  /* [[1, ..., 1], x, ...: the inner array declares 17 items, but the input
   * cannot hold that many if the outer one is to have two more, so it starts
   * smaller and grows */
  unsigned char grown_array[20] = {0x83, 0x91};
  memset(grown_array + 2, 0x01, 17);
  grown_array[19] = 0xFF;
  assert_truncated(grown_array, 19);
  assert_load_error(grown_array, 20, CBOR_ERR_SYNTAXERROR, 20);
  /* The same for a map declaring 17 pairs */
  unsigned char grown_map[37] = {0x83, 0xB1};
  memset(grown_map + 2, 0x01, 34);
  grown_map[36] = 0xFF;
  assert_truncated(grown_map, 36);
  assert_load_error(grown_map, 37, CBOR_ERR_SYNTAXERROR, 37);
}

static void test_growth_failure(void** _state _CBOR_UNUSED) {
  struct cbor_load_result res;
  /* [[1, ...: no storage is preallocated for the inner array */
  const unsigned char grown_array[] = {0x98, 0x20, 0x91, 0x01};
  WITH_MOCK_MALLOC(
      {
        assert_null(cbor_load(grown_array, sizeof(grown_array), &res));
        assert_true(res.error.code == CBOR_ERR_MEMERROR);
        assert_size_equal(res.error.position, 4);
      },
      // Outer array (item, storage, stack record), inner array (item, stack
      // record), the first item, and making room for it
      7, MALLOC, MALLOC, MALLOC, MALLOC, MALLOC, MALLOC, REALLOC_FAIL);

  const unsigned char grown_map[] = {0x98, 0x20, 0xB1, 0x01};
  WITH_MOCK_MALLOC(
      {
        assert_null(cbor_load(grown_map, sizeof(grown_map), &res));
        assert_true(res.error.code == CBOR_ERR_MEMERROR);
        assert_size_equal(res.error.position, 4);
      },
      // The same for a map and its first key
      7, MALLOC, MALLOC, MALLOC, MALLOC, MALLOC, MALLOC, REALLOC_FAIL);
}

/* Small declared sizes are preallocated without looking at the input. Each
 * of these maps declares 16 pairs and is the first key of the one before. */
static void test_small_claims(void** _state _CBOR_UNUSED) {
  unsigned char data[1000];
  memset(data, 0xB0, sizeof(data));
  assert_truncated(data, sizeof(data));
  /* Each byte opens one container, with storage for its declared pairs */
  assert_true(largest_request <= 16 * sizeof(struct cbor_pair));
  assert_true(total_requested <=
              sizeof(data) *
                  (16 * sizeof(struct cbor_pair) + sizeof(cbor_item_t) +
                   sizeof(struct _cbor_stack_record)));
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
      cmocka_unit_test(test_large_valid_containers),
      cmocka_unit_test(test_oversized_claims),
#ifdef EIGHT_BYTE_SIZE_T
      cmocka_unit_test(test_oversized_claims_64),
#endif
      cmocka_unit_test(test_chained_claims),
      cmocka_unit_test(test_same_errors),
      cmocka_unit_test(test_growth_failure),
      cmocka_unit_test(test_small_claims),
      cmocka_unit_test(test_unknown_input_size),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
