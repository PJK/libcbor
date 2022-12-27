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

#include "cbor.h"
#include "cbor/internal/builder_callbacks.h"
#include "cbor/internal/stack.h"

static void test_builder_byte_string_callback_append(
    void **_CBOR_UNUSED(_state)) {
  struct _cbor_stack stack = _cbor_stack_init();
  struct _cbor_decoder_context context = {
      .creation_failed = false,
      .syntax_error = false,
      .root = cbor_new_indefinite_bytestring(),
      .stack = &stack,
  };
  cbor_item_t *chunk = cbor_build_bytestring((cbor_data) "Hello!", 6);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_builder_byte_string_callback_append),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
