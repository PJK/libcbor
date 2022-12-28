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
#include "test_allocator.h"

#include "cbor.h"

// This test simulates cases when malloc unexpectedly fails and leaves a
// possibly partially constructed object behind. It is especially useful
// in conjunction with the memory correctness check.

int main(void) {
  // TODO: string chunks realloc test
  const struct CMUnitTest tests[] = {};

  return cmocka_run_group_tests(tests, NULL, NULL);
}
