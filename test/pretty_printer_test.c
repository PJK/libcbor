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
  cbor_describe(item, stdout);
  rewind(outfile);
  for (size_t i = 0; i < lines; i++) {
    // Expected line + linebreak + null character
    char *buffer = malloc(strlen(result[i]) + 2);
    char *result_with_newline = malloc(strlen(result[i]) + 2);
    assert_non_null(buffer);
    assert_non_null(result_with_newline);
    sprintf(result_with_newline, "%s\n", result[i]);
    fgets(buffer, strlen(result[i]) + 2, outfile);
    fprintf(stdout, "%s", buffer);
    assert_int_equal(strlen(buffer), strlen(result_with_newline));
    assert_string_equal(buffer, result_with_newline);
    free(buffer);
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

int main(void) {
  const struct CMUnitTest tests[] = {cmocka_unit_test(test_uint),
                                     cmocka_unit_test(test_negint),
                                     cmocka_unit_test(test_definite_array)};
  return cmocka_run_group_tests(tests, NULL, NULL);
}
