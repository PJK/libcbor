/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#include "assertions.h"
#include "cbor.h"

unsigned char buffer[512];

static void test_embedded_tag(void** _state _CBOR_UNUSED) {
  assert_size_equal(1, cbor_encode_tag(1, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xC1}), 1);
}

static void test_tag(void** _state _CBOR_UNUSED) {
  assert_size_equal(5, cbor_encode_tag(1000000, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xDA, 0x00, 0x0F, 0x42, 0x40}),
                      5);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_embedded_tag),
      cmocka_unit_test(test_tag),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
