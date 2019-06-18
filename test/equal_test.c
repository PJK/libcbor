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

static void test_equal_array(void **state) {
  cbor_item_t * element;
  
  item1 = cbor_new_definite_array(3);
  item2 = cbor_new_definite_array(3);
  
  element = cbor_build_string("test1");
  cbor_array_push(item1, element);
  cbor_decref(&element);
  
  element = cbor_build_string("test1");
  cbor_array_push(item2, element);
  cbor_decref(&element);
  
  element = cbor_build_negint8(10);
  cbor_array_push(item1, element);
  cbor_decref(&element);
  
  element = cbor_build_negint8(10);
  cbor_array_push(item2, element);
  cbor_decref(&element);
  
  element = cbor_build_bytestring("test", 4);
  cbor_array_push(item1, element);
  cbor_decref(&element);
  
  element = cbor_build_bytestring("test", 4);
  cbor_array_push(item2, element);
  cbor_decref(&element);
  
  assert_true(cbor_equal(item1, item2) == true);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_new_definite_array(3);
  item2 = cbor_new_definite_array(3);
  
  element = cbor_build_negint8(10);
  cbor_array_push(item1, element);
  cbor_decref(&element);
  
  element = cbor_build_string("test1");
  cbor_array_push(item2, element);
  cbor_decref(&element);
  
  element = cbor_build_string("test1");
  cbor_array_push(item1, element);
  cbor_decref(&element);
  
  element = cbor_build_negint8(10);
  cbor_array_push(item2, element);
  cbor_decref(&element);
  
  element = cbor_build_bytestring("test", 4);
  cbor_array_push(item1, element);
  cbor_decref(&element);
  
  element = cbor_build_bytestring("test", 4);
  cbor_array_push(item2, element);
  cbor_decref(&element);
  
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_new_definite_array(3);
  item2 = cbor_new_definite_array(2);
  
  element = cbor_build_string("test1");
  cbor_array_push(item1, element);
  cbor_decref(&element);
  
  element = cbor_build_string("test1");
  cbor_array_push(item2, element);
  cbor_decref(&element);
  
  element = cbor_build_negint8(10);
  cbor_array_push(item1, element);
  cbor_decref(&element);
  
  element = cbor_build_negint8(10);
  cbor_array_push(item2, element);
  cbor_decref(&element);
  
  element = cbor_build_bytestring("test", 4);
  cbor_array_push(item1, element);
  cbor_decref(&element);
  
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
}

static void test_equal_map(void **state) {
  struct cbor_pair pair;
  
  item1 = cbor_new_definite_map(3);
  item2 = cbor_new_definite_map(3);
  
  pair.key = cbor_build_string("key1");
  pair.value = cbor_build_string("value1");
  assert_true(cbor_map_add(item1, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_string("key1");
  pair.value = cbor_build_string("value1");
  assert_true(cbor_map_add(item2, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_negint8(10);
  pair.value = cbor_build_string("value2");
  assert_true(cbor_map_add(item1, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_negint8(10);
  pair.value = cbor_build_string("value2");
  assert_true(cbor_map_add(item2, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_uint8(4);
  pair.value = cbor_build_bytestring("test", 4);
  assert_true(cbor_map_add(item1, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_uint8(4);
  pair.value = cbor_build_bytestring("test", 4);
  assert_true(cbor_map_add(item2, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  assert_true(cbor_equal(item1, item2) == true);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_new_definite_map(3);
  item2 = cbor_new_definite_map(3);
  
  pair.key = cbor_build_string("key1");
  pair.value = cbor_build_string("value1");
  assert_true(cbor_map_add(item1, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_string("key1");
  pair.value = cbor_build_string("value2");
  assert_true(cbor_map_add(item2, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_negint8(10);
  pair.value = cbor_build_string("value2");
  assert_true(cbor_map_add(item1, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_negint8(10);
  pair.value = cbor_build_string("value2");
  assert_true(cbor_map_add(item2, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_uint8(4);
  pair.value = cbor_build_bytestring("test", 4);
  assert_true(cbor_map_add(item1, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_uint8(4);
  pair.value = cbor_build_bytestring("test", 4);
  assert_true(cbor_map_add(item2, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_new_definite_map(3);
  item2 = cbor_new_definite_map(3);
  
  pair.key = cbor_build_string("key1");
  pair.value = cbor_build_string("value1");
  assert_true(cbor_map_add(item1, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_string("key1");
  pair.value = cbor_build_string("value1");
  assert_true(cbor_map_add(item2, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_negint8(10);
  pair.value = cbor_build_string("value2");
  assert_true(cbor_map_add(item1, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_negint8(10);
  pair.value = cbor_build_string("value2");
  assert_true(cbor_map_add(item2, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_uint8(4);
  pair.value = cbor_build_bytestring("test", 4);
  assert_true(cbor_map_add(item1, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_uint8(4);
  pair.value = cbor_build_string("test");
  assert_true(cbor_map_add(item2, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
  
  item1 = cbor_new_definite_map(3);
  item2 = cbor_new_definite_map(3);
  
  pair.key = cbor_build_negint8(10);
  pair.value = cbor_build_string("value2");
  assert_true(cbor_map_add(item1, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_string("key1");
  pair.value = cbor_build_string("value1");
  assert_true(cbor_map_add(item2, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_string("key1");
  pair.value = cbor_build_string("value1");
  assert_true(cbor_map_add(item1, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_negint8(10);
  pair.value = cbor_build_string("value2");
  assert_true(cbor_map_add(item2, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_uint8(4);
  pair.value = cbor_build_bytestring("test", 4);
  assert_true(cbor_map_add(item1, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  pair.key = cbor_build_uint8(4);
  pair.value = cbor_build_bytestring("test", 4);
  assert_true(cbor_map_add(item2, pair) == true);
  cbor_decref(&pair.key);
  cbor_decref(&pair.value);
  
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
}

static void test_equal_tag(void **state) {
  cbor_item_t * tag1, * tag2;
  
  tag1 = cbor_new_tag(8);
  tag2 = cbor_new_tag(8);
  
  assert_true(cbor_equal(tag1, tag2) == true);
  
  tag1 = cbor_new_tag(6);
  tag2 = cbor_new_tag(8);
  
  assert_true(cbor_equal(tag1, tag2) == false);
  
  tag1 = cbor_new_tag(6);
  tag2 = cbor_build_bytestring("test", 4);
  
  assert_true(cbor_equal(tag1, tag2) == false);
  cbor_decref(&tag2);
}

static void test_equal_float_ctrl(void **state) {
  item1 = cbor_build_bool(true);
  item2 = cbor_build_bool(true);
  assert_true(cbor_equal(item1, item2) == true);
  cbor_decref(&item1);
  cbor_decref(&item2);

  item1 = cbor_build_float2(8);
  item2 = cbor_build_float2(8);
  assert_true(cbor_equal(item1, item2) == true);
  cbor_decref(&item1);
  cbor_decref(&item2);

  item1 = cbor_build_float8(1234567890);
  item2 = cbor_build_float8(1234567890);
  assert_true(cbor_equal(item1, item2) == true);
  cbor_decref(&item1);
  cbor_decref(&item2);

  item1 = cbor_new_null();
  item2 = cbor_new_null();
  assert_true(cbor_equal(item1, item2) == true);
  cbor_decref(&item1);
  cbor_decref(&item2);

  item1 = cbor_new_undef();
  item2 = cbor_new_undef();
  assert_true(cbor_equal(item1, item2) == true);
  cbor_decref(&item1);
  cbor_decref(&item2);

  item1 = cbor_build_float2(8);
  item2 = cbor_build_float8(8);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);

  item1 = cbor_new_undef();
  item2 = cbor_new_null();
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);

  item1 = cbor_build_float2(8);
  item2 = cbor_build_bytestring("test", 4);
  assert_true(cbor_equal(item1, item2) == false);
  cbor_decref(&item1);
  cbor_decref(&item2);
}

int main(void) {
  const struct CMUnitTest tests[] = {

      cmocka_unit_test(test_equal_uint),
      cmocka_unit_test(test_equal_negint),
      cmocka_unit_test(test_equal_bytestring),
      cmocka_unit_test(test_equal_string),
      cmocka_unit_test(test_equal_array),
      cmocka_unit_test(test_equal_map),
      cmocka_unit_test(test_equal_tag),
      cmocka_unit_test(test_equal_float_ctrl)
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
