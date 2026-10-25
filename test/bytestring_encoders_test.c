/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#include "assertions.h"

#include "cbor.h"

unsigned char buffer[512];

static void test_embedded_bytestring_start(void** _state _CBOR_UNUSED) {
  assert_size_equal(1, cbor_encode_bytestring_start(1, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0x41}), 1);
}

static void test_bytestring_start(void** _state _CBOR_UNUSED) {
  assert_size_equal(5, cbor_encode_bytestring_start(1000000, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0x5A, 0x00, 0x0F, 0x42, 0x40}),
                      5);
}

static void test_indef_bytestring_start(void** _state _CBOR_UNUSED) {
  assert_size_equal(0, cbor_encode_indef_bytestring_start(buffer, 0));
  assert_size_equal(1, cbor_encode_indef_bytestring_start(buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0x5F}), 1);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_embedded_bytestring_start),
      cmocka_unit_test(test_bytestring_start),
      cmocka_unit_test(test_indef_bytestring_start)};
  return cmocka_run_group_tests(tests, NULL, NULL);
}
