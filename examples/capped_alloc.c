/*
 * Copyright (c) 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include "cbor.h"

/*
 * Illustrates how to use cbor_set_allocs to cap the memory that cbor_load
 * may consume when parsing untrusted data.
 *
 * cbor_load pre-allocates storage for definite-length arrays and maps sized
 * by the element count declared in the CBOR header. The declared count can
 * be up to 2^64-1, so cbor_load will attempt to allocate whatever the
 * header says before reading any element data.
 *
 * Installing a capping malloc via cbor_set_allocs causes cbor_load to
 * return CBOR_ERR_MEMERROR instead of attempting an oversized allocation.
 *
 * Usage: ./capped_alloc [input file]
 * Example: ./capped_alloc examples/data/nested_array.cbor
 */

/*
 * Total memory budget for all libcbor allocations combined. Adjust to suit
 * your application and expected input size.
 *
 * Note: this global counter is not thread-safe. If you use libcbor from
 * multiple threads, guard it with a mutex or use a per-thread budget.
 */
#define MAX_TOTAL_SIZE (8 * 1024 * 1024) /* 8 MiB */

static size_t allocated_bytes = 0;

static void* capping_malloc(size_t size) {
  if (size > MAX_TOTAL_SIZE - allocated_bytes) return NULL;
  void* ptr = malloc(size);
  if (ptr != NULL) allocated_bytes += size;
  return ptr;
}

static void* capping_realloc(void* ptr, size_t size) {
  /* We don't track the old size, so treat realloc conservatively:
   * only check that size fits within the remaining budget. */
  if (size > MAX_TOTAL_SIZE - allocated_bytes) return NULL;
  void* new_ptr = realloc(ptr, size);
  if (new_ptr != NULL) allocated_bytes += size;
  return new_ptr;
}

static void capping_free(void* ptr) {
  free(ptr);
  /* We don't track individual allocation sizes, so we cannot subtract from
   * allocated_bytes on free. The budget therefore measures peak usage, not
   * current live bytes. */
}

void usage(void) {
  printf("Usage: capped_alloc [input file]\n");
  exit(1);
}

int main(int argc, char* argv[]) {
  if (argc != 2) usage();

  /* Install the capping allocator before any cbor_load calls. */
  cbor_set_allocs(capping_malloc, capping_realloc, capping_free);

  FILE* f = fopen(argv[1], "rb");
  if (f == NULL) usage();
  fseek(f, 0, SEEK_END);
  size_t length = (size_t)ftell(f);
  fseek(f, 0, SEEK_SET);

  /* Read the raw input with the system malloc -- the cap applies to
   * libcbor's internal allocations, not to our own buffer. */
  unsigned char* buffer = malloc(length);
  if (buffer == NULL || fread(buffer, length, 1, f) != 1) {
    fprintf(stderr, "Failed to read input\n");
    exit(1);
  }
  fclose(f);

  struct cbor_load_result result;
  cbor_item_t* item = cbor_load(buffer, length, &result);
  free(buffer);

  if (result.error.code != CBOR_ERR_NONE) {
    fprintf(stderr,
            "Failed to decode CBOR near byte %zu: ", result.error.position);
    switch (result.error.code) {
      case CBOR_ERR_MEMERROR:
        fprintf(stderr,
                "allocation failed or exceeded the %d MiB total budget\n",
                MAX_TOTAL_SIZE / (1024 * 1024));
        break;
      case CBOR_ERR_NODATA:
        fprintf(stderr, "empty input\n");
        break;
      case CBOR_ERR_NOTENOUGHDATA:
        fprintf(stderr, "truncated input\n");
        break;
      case CBOR_ERR_MALFORMATED:
        fprintf(stderr, "malformed input\n");
        break;
      case CBOR_ERR_SYNTAXERROR:
        fprintf(stderr, "syntax error\n");
        break;
      case CBOR_ERR_NONE:
        break;
    }
    exit(1);
  }

  cbor_describe(item, stdout);
  fflush(stdout);
  cbor_decref(&item);
}
