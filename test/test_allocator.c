#include "test_allocator.h"

// How many alloc calls we expect
int alloc_calls_expected;
// How many alloc calls we got
int alloc_calls;
// Array of booleans indicating whether to return a block or fail with NULL
call_expectation *expectations;

void set_mock_malloc(int calls, ...) {
  va_list args;
  va_start(args, calls);
  alloc_calls_expected = calls;
  alloc_calls = 0;
  expectations = calloc(calls, sizeof(expectations));
  for (int i = 0; i < calls; i++) {
    // Promotable types, baby
    expectations[i] = va_arg(args, call_expectation);
  }
  va_end(args);
}

void finalize_mock_malloc(void) {
  assert_int_equal(alloc_calls, alloc_calls_expected);
  free(expectations);
}

void *instrumented_malloc(size_t size) {
  if (alloc_calls >= alloc_calls_expected) {
    goto error;
  }

  if (expectations[alloc_calls] == MALLOC) {
    alloc_calls++;
    return malloc(size);
  } else if (expectations[alloc_calls] == MALLOC_FAIL) {
    alloc_calls++;
    return NULL;
  }

error:
  print_error("Unexpected call to malloc");
  fail();
  return NULL;
}

void *instrumented_realloc(void *ptr, size_t size) {
  if (alloc_calls >= alloc_calls_expected) {
    goto error;
  }

  if (expectations[alloc_calls] == REALLOC) {
    alloc_calls++;
    return realloc(ptr, size);
  } else if (expectations[alloc_calls] == REALLOC_FAIL) {
    alloc_calls++;
    return NULL;
  }

error:
  print_error("Unexpected call to realloc");
  fail();
  return NULL;
}
