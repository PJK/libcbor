#ifndef TEST_ALLOCATOR_H_
#define TEST_ALLOCATOR_H_

#include "assertions.h"
#include "cbor.h"

// Harness for mocking `malloc` and `realloc`

typedef enum call_expectation {
  // Call malloc and return a pointer
  MALLOC,
  // Pretend call malloc, but return NULL (fail)
  MALLOC_FAIL,
  // Call realloc and return a pointer
  REALLOC,
  // Pretend call realloc, but return NULL (fail)
  REALLOC_FAIL
} call_expectation;

void set_mock_malloc(int calls, ...);

void finalize_mock_malloc(void);

void* instrumented_malloc(size_t size);

void* instrumented_realloc(void* ptr, size_t size);

#define WITH_MOCK_MALLOC(block, malloc_calls, ...)                    \
  do {                                                                \
    cbor_set_allocs(instrumented_malloc, instrumented_realloc, free); \
    set_mock_malloc(malloc_calls, __VA_ARGS__);                       \
    block;                                                            \
    finalize_mock_malloc();                                           \
    cbor_set_allocs(malloc, realloc, free);                           \
  } while (0)

#define WITH_FAILING_MALLOC(block) WITH_MOCK_MALLOC(block, 1, MALLOC_FAIL)

// malloc(0) is allowed to return NULL (C99 7.20.3). Some platforms and custom
// allocators do so; this simulates that behavior.
void* malloc_null_for_zero_size(size_t size);

#define WITH_MALLOC_NULL_FOR_ZERO_SIZE(block)                  \
  do {                                                         \
    cbor_set_allocs(malloc_null_for_zero_size, realloc, free); \
    block;                                                     \
    cbor_set_allocs(malloc, realloc, free);                    \
  } while (0)

#endif  // TEST_ALLOCATOR_H_
