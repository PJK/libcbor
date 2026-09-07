/*
 * Copyright (c) 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <time.h>
#include "assertions.h"
#include "cbor.h"

#ifdef HUGE_FUZZ
#define ROUNDS 65536ULL
#define MAXLEN 131072ULL
#else
#define ROUNDS 256ULL
#define MAXLEN 2048ULL
#endif

#ifdef PRINT_FUZZ
static void printmem(const unsigned char* ptr, size_t length) {
  for (size_t i = 0; i < length; i++) printf("%02X", ptr[i]);
  printf("\n");
}
#endif

unsigned seed;
static size_t allocation_count;
static size_t fail_at;
static size_t live_allocations;

static bool fail_allocation(size_t size) {
  allocation_count++;
  return allocation_count == fail_at || size > (1 << 19);
}

static void* mock_malloc(size_t size) {
  if (fail_allocation(size)) return NULL;
  void* ptr = malloc(size);
  if (ptr != NULL) live_allocations++;
  return ptr;
}

static void* mock_realloc(void* ptr, size_t size) {
  if (fail_allocation(size)) return NULL;
  // The decoder only requests nonzero realloc sizes.
  assert_true(size > 0);
  bool new_allocation = ptr == NULL;
  void* result = realloc(ptr, size);
  if (result != NULL && new_allocation) live_allocations++;
  return result;
}

static void mock_free(void* ptr) {
  if (ptr != NULL) {
    assert_true(live_allocations > 0);
    live_allocations--;
  }
  free(ptr);
}

static cbor_error_code load_with_failure(cbor_data data, size_t length,
                                         size_t failure) {
  allocation_count = 0;
  fail_at = failure;
  struct cbor_load_result res;
  cbor_item_t* item = cbor_load(data, length, &res);
  if (failure != 0) {
    assert_true(allocation_count >= failure);
    assert_int_equal(res.error.code, CBOR_ERR_MEMERROR);
  }
  if (res.error.code == CBOR_ERR_NONE) {
    assert_non_null(item);
    cbor_decref(&item);
  } else {
    assert_null(item);
  }
  assert_size_equal(live_allocations, 0);
  return res.error.code;
}

static void run_input(cbor_data data, size_t length) {
  // Count the allocations on the original path, then replay the same input
  // with each malloc/realloc failing in turn. No PRNG calls during replay.
  load_with_failure(data, length, 0);
  size_t allocations = allocation_count;
  for (size_t failure = 1; failure <= allocations; failure++) {
    load_with_failure(data, length, failure);
  }
}

static void run_round(void) {
  size_t length = rand() % MAXLEN + 1;
  unsigned char* data = malloc(length);
  for (size_t i = 0; i < length; i++) {
    data[i] = rand() % 0xFF;
  }

#ifdef PRINT_FUZZ
  printmem(data, length);
#endif

  run_input(data, length);

  free(data);
}

static void fuzz(void** _state _CBOR_UNUSED) {
  cbor_set_allocs(mock_malloc, mock_realloc, mock_free);
  printf("Fuzzing %llu rounds of up to %llu bytes with seed %u\n", ROUNDS,
         MAXLEN, seed);
  srand(seed);

  for (size_t i = 0; i < ROUNDS; i++) run_round();

  cbor_set_allocs(malloc, realloc, free);

  printf("Successfully fuzzed through %llu kB of data\n",
         (ROUNDS * MAXLEN) / 1024);
}

static void fuzz_nested_allocations(void** _state _CBOR_UNUSED) {
  // Valid nested input ensures that realloc and cleanup of partially built
  // maps, arrays, tags, and chunked strings are exercised on every invocation.
  const unsigned char data[] = {
      0x9f,                                // Indefinite array
      0xa1, 0x01, 0x82, 0x02, 0x03,        // Definite map and array
      0xbf, 0x04, 0xc0, 0x05, 0xff,        // Indefinite map and tag
      0x5f, 0x41, 0x01, 0x41, 0x02, 0xff,  // Chunked byte string
      0x7f, 0x61, 0x61, 0x61, 0x62, 0xff,  // Chunked text string
      0xff};
  cbor_set_allocs(mock_malloc, mock_realloc, mock_free);
  assert_int_equal(load_with_failure(data, sizeof(data), 0), CBOR_ERR_NONE);
  run_input(data, sizeof(data));
  cbor_set_allocs(malloc, realloc, free);
}

int main(int argc, char* argv[]) {
  if (argc > 1)
    seed = (unsigned)strtoul(argv[1], NULL, 10);
  else
    seed = (unsigned)time(NULL);

  const struct CMUnitTest tests[] = {cmocka_unit_test(fuzz_nested_allocations),
                                     cmocka_unit_test(fuzz)};
  return cmocka_run_group_tests(tests, NULL, NULL);
}
