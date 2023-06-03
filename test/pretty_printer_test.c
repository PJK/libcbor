/*
 * Copyright (c) 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <string.h>

#include "assertions.h"
#include "cbor.h"

void assert_describe_result(cbor_item_t *item, char *result[], size_t lines) {
#if CBOR_PRETTY_PRINTER
  FILE *outfile = tmpfile();
  cbor_describe(item, outfile);
  rewind(outfile);
  for (size_t i = 0; i < lines; i++) {
    // Expected line + linebreak + null character
    size_t buffer_size = strlen(result[i]) + 2;
    char *buffer = malloc(buffer_size);
    char *result_with_newline = malloc(buffer_size);
    assert_non_null(buffer);
    assert_non_null(result_with_newline);
    snprintf(result_with_newline, buffer_size, "%s\n", result[i]);
    fgets(buffer, strlen(result[i]) + 2, outfile);
    assert_int_equal(strlen(buffer), strlen(result_with_newline));
    assert_string_equal(buffer, result_with_newline);
    free(buffer);
    free(result_with_newline);
  }
  fgetc(outfile);
  assert_true(feof(outfile));
  fclose(outfile);
#endif
}

static void test_uint(void **_CBOR_UNUSED(_state)) {
  cbor_item_t *item = cbor_build_uint8(42);
  char *expected[] = {"[CBOR_TYPE_UINT] Width: 1B, Value: 42"};
  assert_describe_result(item, expected, 1);
  cbor_decref(&item);
}

static void test_negint(void **_CBOR_UNUSED(_state)) {
  cbor_item_t *item = cbor_build_negint16(40);
  char *expected[] = {"[CBOR_TYPE_NEGINT] Width: 2B, Value: -40 - 1"};
  assert_describe_result(item, expected, 1);
  cbor_decref(&item);
}

static void test_definite_bytestring(void **_CBOR_UNUSED(_state)) {
  unsigned char data[] = {0x01, 0x02, 0x03};
  cbor_item_t *item = cbor_build_bytestring(data, 3);
  char *expected[] = {
      "[CBOR_TYPE_BYTESTRING] Definite, length 3B",
      "    010203",
  };
  assert_describe_result(item, expected, 2);
  cbor_decref(&item);
}

static void test_indefinite_bytestring(void **_CBOR_UNUSED(_state)) {
  unsigned char data[] = {0x01, 0x02, 0x03};
  cbor_item_t *item = cbor_new_indefinite_bytestring();
  assert_true(cbor_bytestring_add_chunk(
      item, cbor_move(cbor_build_bytestring(data, 3))));
  assert_true(cbor_bytestring_add_chunk(
      item, cbor_move(cbor_build_bytestring(data, 2))));
  char *expected[] = {
      "[CBOR_TYPE_BYTESTRING] Indefinite, with 2 chunks:",
      "    [CBOR_TYPE_BYTESTRING] Definite, length 3B",
      "        010203",
      "    [CBOR_TYPE_BYTESTRING] Definite, length 2B",
      "        0102",
  };
  assert_describe_result(item, expected, 5);
  cbor_decref(&item);
}

static void test_definite_string(void **_CBOR_UNUSED(_state)) {
  char *string = "Hello!";
  cbor_item_t *item = cbor_build_string(string);
  char *expected[] = {
      // TODO: Codepoint metadata is not set by the builder, fix.
      "[CBOR_TYPE_STRING] Definite, length 6B, 0 codepoints",
      "    Hello!",
  };
  assert_describe_result(item, expected, 2);
  cbor_decref(&item);
}

static void test_indefinite_string(void **_CBOR_UNUSED(_state)) {
  char *string = "Hello!";
  cbor_item_t *item = cbor_new_indefinite_string();
  assert_true(
      cbor_string_add_chunk(item, cbor_move(cbor_build_string(string))));
  assert_true(
      cbor_string_add_chunk(item, cbor_move(cbor_build_string(string))));
  char *expected[] = {
      "[CBOR_TYPE_STRING] Indefinite, with 2 chunks:",
      "    [CBOR_TYPE_STRING] Definite, length 6B, 0 codepoints",
      "        Hello!",
      "    [CBOR_TYPE_STRING] Definite, length 6B, 0 codepoints",
      "        Hello!",
  };
  assert_describe_result(item, expected, 5);
  cbor_decref(&item);
}

static void test_definite_array(void **_CBOR_UNUSED(_state)) {
  cbor_item_t *item = cbor_new_definite_array(2);
  assert_true(cbor_array_push(item, cbor_move(cbor_build_uint8(1))));
  assert_true(cbor_array_push(item, cbor_move(cbor_build_uint8(2))));
  char *expected[] = {
      "[CBOR_TYPE_ARRAY] Definite, size: 2",
      "    [CBOR_TYPE_UINT] Width: 1B, Value: 1",
      "    [CBOR_TYPE_UINT] Width: 1B, Value: 2",
  };
  assert_describe_result(item, expected, 3);
  cbor_decref(&item);
}

static void test_indefinite_array(void **_CBOR_UNUSED(_state)) {
  cbor_item_t *item = cbor_new_indefinite_array();
  assert_true(cbor_array_push(item, cbor_move(cbor_build_uint8(1))));
  assert_true(cbor_array_push(item, cbor_move(cbor_build_uint8(2))));
  char *expected[] = {
      "[CBOR_TYPE_ARRAY] Indefinite, size: 2",
      "    [CBOR_TYPE_UINT] Width: 1B, Value: 1",
      "    [CBOR_TYPE_UINT] Width: 1B, Value: 2",
  };
  assert_describe_result(item, expected, 3);
  cbor_decref(&item);
}

static void test_definite_map(void **_CBOR_UNUSED(_state)) {
  cbor_item_t *item = cbor_new_definite_map(1);
  assert_true(cbor_map_add(
      item, (struct cbor_pair){.key = cbor_move(cbor_build_uint8(1)),
                               .value = cbor_move(cbor_build_uint8(2))}));
  char *expected[] = {
      "[CBOR_TYPE_MAP] Definite, size: 1",
      "    [CBOR_TYPE_UINT] Width: 1B, Value: 1",
      "    [CBOR_TYPE_UINT] Width: 1B, Value: 2",
  };
  assert_describe_result(item, expected, 3);
  cbor_decref(&item);
}

static void test_indefinite_map(void **_CBOR_UNUSED(_state)) {
  cbor_item_t *item = cbor_new_indefinite_map();
  assert_true(cbor_map_add(
      item, (struct cbor_pair){.key = cbor_move(cbor_build_uint8(1)),
                               .value = cbor_move(cbor_build_uint8(2))}));
  char *expected[] = {
      "[CBOR_TYPE_MAP] Indefinite, size: 1",
      "    [CBOR_TYPE_UINT] Width: 1B, Value: 1",
      "    [CBOR_TYPE_UINT] Width: 1B, Value: 2",
  };
  assert_describe_result(item, expected, 3);
  cbor_decref(&item);
}

static void test_tag(void **_CBOR_UNUSED(_state)) {
  cbor_item_t *item = cbor_build_tag(42, cbor_move(cbor_build_uint8(1)));
  char *expected[] = {
      "[CBOR_TYPE_TAG] Value: 42",
      "    [CBOR_TYPE_UINT] Width: 1B, Value: 1",
  };
  assert_describe_result(item, expected, 2);
  cbor_decref(&item);
}

static void test_floats(void **_CBOR_UNUSED(_state)) {
  cbor_item_t *item = cbor_new_indefinite_array();
  assert_true(cbor_array_push(item, cbor_move(cbor_build_bool(true))));
  assert_true(
      cbor_array_push(item, cbor_move(cbor_build_ctrl(CBOR_CTRL_UNDEF))));
  assert_true(
      cbor_array_push(item, cbor_move(cbor_build_ctrl(CBOR_CTRL_NULL))));
  assert_true(cbor_array_push(item, cbor_move(cbor_build_ctrl(24))));
  assert_true(cbor_array_push(item, cbor_move(cbor_build_float4(3.14f))));
  char *expected[] = {
      "[CBOR_TYPE_ARRAY] Indefinite, size: 5",
      "    [CBOR_TYPE_FLOAT_CTRL] Bool: true",
      "    [CBOR_TYPE_FLOAT_CTRL] Undefined",
      "    [CBOR_TYPE_FLOAT_CTRL] Null",
      "    [CBOR_TYPE_FLOAT_CTRL] Simple value: 24",
      "    [CBOR_TYPE_FLOAT_CTRL] Width: 4B, value: 3.140000",
  };
  assert_describe_result(item, expected, 6);
  cbor_decref(&item);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_uint),
      cmocka_unit_test(test_negint),
      cmocka_unit_test(test_definite_bytestring),
      cmocka_unit_test(test_indefinite_bytestring),
      cmocka_unit_test(test_definite_string),
      cmocka_unit_test(test_indefinite_string),
      cmocka_unit_test(test_definite_array),
      cmocka_unit_test(test_indefinite_array),
      cmocka_unit_test(test_definite_map),
      cmocka_unit_test(test_indefinite_map),
      cmocka_unit_test(test_tag),
      cmocka_unit_test(test_floats),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
