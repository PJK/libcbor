/*
 * Copyright (c) 2014-2019 Pavel Kalvoda <me@pavelkalvoda.com>
 *                    2019 Nicolas Mora <mail@babelouest.org>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "assertions.h"
#include "cbor.h"

cbor_item_t *item1, *item2;

static void test_equal_uint(void **state) {
  item1 = cbor_build_uint8(10);
  item2 = cbor_build_uint8(10);
  assert_true(cbor_equal(item1, item2) == true);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_uint8(10);
  item2 = cbor_build_uint8(8);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_uint8(10);
  item2 = cbor_build_uint16(10);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_uint8(10);
  item2 = cbor_build_negint8(10);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_uint8(10);
  item2 = cbor_build_bytestring("test", 4);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
}

static void test_equal_negint(void **state) {
  item1 = cbor_build_negint8(10);
  item2 = cbor_build_negint8(10);
  assert_true(cbor_equal(item1, item2) == true);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_negint8(10);
  item2 = cbor_build_negint8(8);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_negint8(10);
  item2 = cbor_build_negint16(10);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_negint8(10);
  item2 = cbor_build_uint8(10);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_negint8(10);
  item2 = cbor_build_bytestring("test", 4);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
}

static void test_equal_bytestring(void **state) {
  item1 = cbor_build_bytestring("test", 4);
  item2 = cbor_build_bytestring("test", 4);
  assert_true(cbor_equal(item1, item2) == true);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_bytestring("test", 4);
  item2 = cbor_build_bytestring("tes", 3);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_bytestring("test", 4);
  item2 = cbor_build_bytestring("tset", 4);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_bytestring("test", 4);
  item2 = cbor_build_negint8(10);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
}

static void test_equal_string(void **state) {
  item1 = cbor_build_string("test");
  item2 = cbor_build_string("test");
  assert_true(cbor_equal(item1, item2) == true);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_string("test");
  item2 = cbor_build_string("tes");
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_string("test");
  item2 = cbor_build_string("tset");
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_build_string("test");
  item2 = cbor_build_negint8(10);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
}

int main(void) {
  const struct CMUnitTest tests[] = {

      cmocka_unit_test(test_equal_uint),
      cmocka_unit_test(test_equal_negint),
      cmocka_unit_test(test_equal_bytestring),
      cmocka_unit_test(test_equal_string)
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
