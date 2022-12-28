/*
 * Copyright (c) 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include <string.h>
#include "assertions.h"
#include "cbor/internal/memory_utils.h"

static void test_safe_multiply(void **_CBOR_UNUSED(_state)) {
  assert_true(_cbor_safe_to_multiply(1, 1));
  assert_true(_cbor_safe_to_multiply(SIZE_MAX, 0));
  assert_true(_cbor_safe_to_multiply(SIZE_MAX, 1));
  assert_false(_cbor_safe_to_multiply(SIZE_MAX, 2));
  assert_false(_cbor_safe_to_multiply(SIZE_MAX, SIZE_MAX));
}

static void test_safe_add(void **_CBOR_UNUSED(_state)) {
  assert_true(_cbor_safe_to_add(1, 1));
  assert_true(_cbor_safe_to_add(SIZE_MAX - 1, 1));
  assert_true(_cbor_safe_to_add(SIZE_MAX, 0));
  assert_false(_cbor_safe_to_add(SIZE_MAX, 1));
  assert_false(_cbor_safe_to_add(SIZE_MAX, 2));
  assert_false(_cbor_safe_to_add(SIZE_MAX, SIZE_MAX));
  assert_false(_cbor_safe_to_add(SIZE_MAX - 1, SIZE_MAX));
  assert_false(_cbor_safe_to_add(SIZE_MAX - 1, SIZE_MAX - 1));
}

static void test_safe_signalling_add(void **_CBOR_UNUSED(_state)) {
  assert_int_equal(_cbor_safe_signaling_add(1, 2), 3);
  assert_int_equal(_cbor_safe_signaling_add(0, 1), 0);
  assert_int_equal(_cbor_safe_signaling_add(0, SIZE_MAX), 0);
  assert_int_equal(_cbor_safe_signaling_add(1, SIZE_MAX), 0);
  assert_int_equal(_cbor_safe_signaling_add(1, SIZE_MAX - 1), SIZE_MAX);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_safe_multiply),
      cmocka_unit_test(test_safe_add),
      cmocka_unit_test(test_safe_signalling_add),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
