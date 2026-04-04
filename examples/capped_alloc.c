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
 * To accurately track current live bytes we need to know the size of each
 * allocation when it is freed or reallocated. The C standard does not expose
 * this, but most platforms provide a query function:
 *
 *   macOS / BSD : malloc_size()   from <malloc/malloc.h>
 *   Linux/glibc : malloc_usable_size() from <malloc.h>
 *
 * When available we use these to subtract the correct amount in free() and to
 * account for the old allocation in realloc(), making allocated_bytes track
 * current live bytes rather than a monotonically increasing peak.
 *
 * Without them we fall back to a conservative approximation: the budget only
 * grows (frees are not subtracted) and realloc adds the new size on top of the
 * old one. The budget therefore over-counts, causing cbor_load to fail sooner
 * than strictly necessary, but it is still a safe upper bound.
 */
#if defined(__APPLE__)
#include <malloc/malloc.h>
#define ALLOC_SIZE(ptr) malloc_size(ptr)
#elif defined(__linux__) && defined(__GLIBC__)
#include <malloc.h>
#define ALLOC_SIZE(ptr) malloc_usable_size(ptr)
#endif

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
#ifdef ALLOC_SIZE
  if (ptr != NULL) allocated_bytes += ALLOC_SIZE(ptr);
#else
  if (ptr != NULL) allocated_bytes += size;
#endif
  return ptr;
}

static void* capping_realloc(void* ptr, size_t size) {
#ifdef ALLOC_SIZE
  /* Subtract the old allocation before checking the budget. */
  size_t old_size = ptr != NULL ? ALLOC_SIZE(ptr) : 0;
  if (size > MAX_TOTAL_SIZE - allocated_bytes + old_size) return NULL;
  void* new_ptr = realloc(ptr, size);
  if (new_ptr != NULL) {
    allocated_bytes -= old_size;
    allocated_bytes += ALLOC_SIZE(new_ptr);
  }
#else
  /* Without ALLOC_SIZE we cannot subtract the old allocation: treat realloc
   * as a fresh allocation added on top of the existing budget. */
  if (size > MAX_TOTAL_SIZE - allocated_bytes) return NULL;
  void* new_ptr = realloc(ptr, size);
  if (new_ptr != NULL) allocated_bytes += size;
#endif
  return new_ptr;
}

static void capping_free(void* ptr) {
#ifdef ALLOC_SIZE
  if (ptr != NULL) allocated_bytes -= ALLOC_SIZE(ptr);
#endif
  free(ptr);
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
