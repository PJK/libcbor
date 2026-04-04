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
 * A CBOR input can declare a definite-length array or map with up to
 * 2^64-1 elements in as few as 9 bytes. cbor_load pre-allocates storage
 * for all declared elements before reading any of them, so a crafted input
 * can request many gigabytes of memory from a tiny payload.
 *
 * Installing a capping malloc via cbor_set_allocs causes cbor_load to
 * return CBOR_ERR_MEMERROR instead of attempting an oversized allocation.
 *
 * Usage: ./capped_alloc [input file]
 * Example: ./capped_alloc examples/data/nested_array.cbor
 */

/*
 * Maximum size for any single allocation made by libcbor. This caps the
 * pre-allocation for definite-length collections: an array or map with N
 * elements pre-allocates N * sizeof(pointer), so this limit constrains
 * the largest collection that can be decoded in one shot.
 *
 * Adjust to suit your application and expected input size.
 */
#define MAX_ALLOC_SIZE (8 * 1024 * 1024) /* 8 MiB */

static void* capping_malloc(size_t size) {
  if (size > MAX_ALLOC_SIZE) return NULL;
  return malloc(size);
}

static void* capping_realloc(void* ptr, size_t size) {
  if (size > MAX_ALLOC_SIZE) return NULL;
  return realloc(ptr, size);
}

void usage(void) {
  printf("Usage: capped_alloc [input file]\n");
  exit(1);
}

int main(int argc, char* argv[]) {
  if (argc != 2) usage();

  /* Install the capping allocator before any cbor_load calls. */
  cbor_set_allocs(capping_malloc, capping_realloc, free);

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
        fprintf(
            stderr,
            "allocation failed or exceeded the %d MiB per-allocation "
            "cap -- input may contain a malicious large-collection header\n",
            MAX_ALLOC_SIZE / (1024 * 1024));
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
