#ifndef TEST_ALLOCATOR_H_
#define TEST_ALLOCATOR_H_

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "cbor.h"

// Harness for mocking `malloc` and `realloc`

typedef enum call_expectation {
  MALLOC,
  MALLOC_FAIL,
  REALLOC,
  REALLOC_FAIL
} call_expectation;

void set_mock_malloc(int calls, ...);

void finalize_mock_malloc(void);

void *instrumented_malloc(size_t size);

void *instrumented_realloc(void *ptr, size_t size);

#define WITH_MOCK_MALLOC(block, malloc_calls, ...) \
  do {                                             \
    set_mock_malloc(malloc_calls, __VA_ARGS__);    \
    block;                                         \
    finalize_mock_malloc();                        \
  } while (0)

#define WITH_FAILING_MALLOC(block) WITH_MOCK_MALLOC(block, 1, MALLOC_FAIL)

#endif  // TEST_ALLOCATOR_H_
