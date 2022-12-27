/*
 * Copyright (c) 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <cmocka.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "cbor.h"
#include "cbor/internal/builder_callbacks.h"
#include "cbor/internal/stack.h"
#include "test_allocator.h"

unsigned char data[] = {
    0x93, 0x01, 0x19, 0x01, 0x01, 0x1A, 0x00, 0x01, 0x05, 0xB8, 0x1B, 0x00,
    0x00, 0x00, 0x01, 0x8F, 0x5A, 0xE8, 0xB8, 0x20, 0x39, 0x01, 0x00, 0x3A,
    0x00, 0x01, 0x05, 0xB7, 0x3B, 0x00, 0x00, 0x00, 0x01, 0x8F, 0x5A, 0xE8,
    0xB7, 0x5F, 0x41, 0x01, 0x41, 0x02, 0xFF, 0x7F, 0x61, 0x61, 0x61, 0x62,
    0xFF, 0x9F, 0xFF, 0xA1, 0x61, 0x61, 0x61, 0x62, 0xC0, 0xBF, 0xFF, 0xF9,
    0x3C, 0x00, 0xFA, 0x47, 0xC3, 0x50, 0x00, 0xFB, 0x7E, 0x37, 0xE4, 0x3C,
    0x88, 0x00, 0x75, 0x9C, 0xF6, 0xF7, 0xF5};

/* Exercise the default callbacks */
static void test_default_callbacks(void **_CBOR_UNUSED(_state)) {
  size_t read = 0;
  while (read < 79) {
    struct cbor_decoder_result result =
        cbor_stream_decode(data + read, 79 - read, &cbor_empty_callbacks, NULL);
    read += result.read;
  }
}

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
      cmocka_unit_test(test_default_callbacks),
      cmocka_unit_test(test_builder_byte_string_callback_append),
  };

  cmocka_run_group_tests(tests, NULL, NULL);
}
