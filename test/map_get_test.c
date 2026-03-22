/*
 * Copyright (c) 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include <cmocka.h>

#include "assertions.h"
#include "cbor.h"

/* -------------------------------------------------------------------------
 * Custom equality functions used in tests
 * ---------------------------------------------------------------------- */

/* Text-string equality that only compares byte content, ignoring encoding
 * width. Suitable for maps whose keys are always text strings. */
static bool str_eq(const cbor_item_t* a, const cbor_item_t* b) {
  if (!cbor_isa_string(a) || !cbor_isa_string(b)) return false;
  if (!cbor_string_is_definite(a) || !cbor_string_is_definite(b)) return false;
  if (cbor_string_length(a) != cbor_string_length(b)) return false;
  return memcmp(cbor_string_handle(a), cbor_string_handle(b),
                cbor_string_length(a)) == 0;
}

/* Integer equality that ignores encoding width: uint8(1) == uint16(1).
 * UINT and NEGINT remain distinct (they have different CBOR major types and
 * therefore different abstract values). Demonstrates that a caller-supplied
 * equality function can implement any desired data-model semantics. */
static bool int_value_eq(const cbor_item_t* a, const cbor_item_t* b) {
  if (!cbor_is_int(a) || !cbor_is_int(b)) return false;
  if (cbor_typeof(a) != cbor_typeof(b)) return false;
  return cbor_get_int(a) == cbor_get_int(b);
}

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

static cbor_item_t* build_map(void) {
  /* {  "one": uint8(1),  "two": uint8(2),  negint8(0): "neg"  } */
  cbor_item_t* map = cbor_new_definite_map(3);
  assert_true(cbor_map_add(
      map, (struct cbor_pair){.key = cbor_move(cbor_build_string("one")),
                              .value = cbor_move(cbor_build_uint8(1))}));
  assert_true(cbor_map_add(
      map, (struct cbor_pair){.key = cbor_move(cbor_build_string("two")),
                              .value = cbor_move(cbor_build_uint8(2))}));
  assert_true(cbor_map_add(
      map, (struct cbor_pair){.key = cbor_move(cbor_build_negint8(0)),
                              .value = cbor_move(cbor_build_string("neg"))}));
  return map;
}

/* -------------------------------------------------------------------------
 * Tests
 * ---------------------------------------------------------------------- */

static void test_get_existing_key(void** state _CBOR_UNUSED) {
  cbor_item_t* map = build_map();

  cbor_item_t* key = cbor_build_string("one");
  cbor_item_t* val = cbor_map_get(map, key, cbor_structurally_equal);
  assert_non_null(val);
  assert_true(cbor_isa_uint(val));
  assert_true(cbor_get_uint8(val) == 1);
  cbor_decref(&key);
  cbor_decref(&val);

  key = cbor_build_string("two");
  val = cbor_map_get(map, key, cbor_structurally_equal);
  assert_non_null(val);
  assert_true(cbor_get_uint8(val) == 2);
  cbor_decref(&key);
  cbor_decref(&val);

  cbor_decref(&map);
}

static void test_get_missing_key(void** state _CBOR_UNUSED) {
  cbor_item_t* map = build_map();

  cbor_item_t* key = cbor_build_string("three");
  cbor_item_t* val = cbor_map_get(map, key, cbor_structurally_equal);
  assert_null(val);
  cbor_decref(&key);

  cbor_decref(&map);
}

static void test_get_empty_map(void** state _CBOR_UNUSED) {
  cbor_item_t* map = cbor_new_definite_map(0);
  cbor_item_t* key = cbor_build_string("x");
  assert_null(cbor_map_get(map, key, cbor_structurally_equal));
  cbor_decref(&key);
  cbor_decref(&map);
}

static void test_get_wrong_type(void** state _CBOR_UNUSED) {
  /* Looking up a string key in a map that has an integer key for the same
   * encoded bytes: structural equality correctly returns NULL. */
  cbor_item_t* map = build_map();
  cbor_item_t* key = cbor_build_uint8(1);
  assert_null(cbor_map_get(map, key, cbor_structurally_equal));
  cbor_decref(&key);
  cbor_decref(&map);
}

static void test_get_encoding_width_matters_with_structural_eq(
    void** state _CBOR_UNUSED) {
  /* With structural equality: uint8(1) and uint16(1) are different keys. */
  cbor_item_t* map = cbor_new_definite_map(1);
  assert_true(cbor_map_add(
      map, (struct cbor_pair){.key = cbor_move(cbor_build_uint8(1)),
                              .value = cbor_move(cbor_build_string("found"))}));

  cbor_item_t* key8 = cbor_build_uint8(1);
  cbor_item_t* val = cbor_map_get(map, key8, cbor_structurally_equal);
  assert_non_null(val);
  cbor_decref(&val);
  cbor_decref(&key8);

  cbor_item_t* key16 = cbor_build_uint16(1);
  assert_null(cbor_map_get(map, key16, cbor_structurally_equal));
  cbor_decref(&key16);

  cbor_decref(&map);
}

static void test_get_custom_eq_ignores_width(void** state _CBOR_UNUSED) {
  /* With int_value_eq: uint8(1) and uint16(1) are equivalent keys,
   * so a map keyed with uint8(1) can be looked up via uint16(1). */
  cbor_item_t* map = cbor_new_definite_map(1);
  assert_true(cbor_map_add(
      map, (struct cbor_pair){.key = cbor_move(cbor_build_uint8(1)),
                              .value = cbor_move(cbor_build_string("found"))}));

  cbor_item_t* key = cbor_build_uint16(1);
  cbor_item_t* val = cbor_map_get(map, key, int_value_eq);
  assert_non_null(val);
  assert_true(cbor_isa_string(val));
  cbor_decref(&val);
  cbor_decref(&key);

  cbor_decref(&map);
}

static void test_get_custom_str_eq(void** state _CBOR_UNUSED) {
  /* str_eq is a type-specific comparator that avoids touching non-string
   * keys at all, demonstrating a lightweight application-specific approach. */
  cbor_item_t* map = build_map();

  cbor_item_t* key = cbor_build_string("two");
  cbor_item_t* val = cbor_map_get(map, key, str_eq);
  assert_non_null(val);
  assert_true(cbor_get_uint8(val) == 2);
  cbor_decref(&val);
  cbor_decref(&key);

  cbor_decref(&map);
}

static void test_get_first_match_on_duplicate_keys(void** state _CBOR_UNUSED) {
  /* RFC 8949 §5.3: a map with duplicate keys is well-formed but not valid.
   * cbor_map_get returns the first matching entry. */
  cbor_item_t* map = cbor_new_definite_map(2);
  assert_true(cbor_map_add(
      map, (struct cbor_pair){.key = cbor_move(cbor_build_string("k")),
                              .value = cbor_move(cbor_build_uint8(1))}));
  assert_true(cbor_map_add(
      map, (struct cbor_pair){.key = cbor_move(cbor_build_string("k")),
                              .value = cbor_move(cbor_build_uint8(2))}));

  cbor_item_t* key = cbor_build_string("k");
  cbor_item_t* val = cbor_map_get(map, key, cbor_structurally_equal);
  assert_non_null(val);
  assert_true(cbor_get_uint8(val) == 1); /* first entry wins */
  cbor_decref(&val);
  cbor_decref(&key);

  cbor_decref(&map);
}

static void test_get_refcount(void** state _CBOR_UNUSED) {
  /* The returned value has its reference count incremented. */
  cbor_item_t* map = build_map();

  cbor_item_t* key = cbor_build_string("one");
  cbor_item_t* val = cbor_map_get(map, key, cbor_structurally_equal);
  assert_non_null(val);
  assert_true(cbor_refcount(val) == 2); /* held by map + returned ref */
  cbor_decref(&val);
  cbor_decref(&key);

  cbor_decref(&map);
}

static void test_get_indefinite_map(void** state _CBOR_UNUSED) {
  cbor_item_t* map = cbor_new_indefinite_map();
  assert_true(cbor_map_add(
      map, (struct cbor_pair){.key = cbor_move(cbor_build_string("x")),
                              .value = cbor_move(cbor_build_uint8(99))}));

  cbor_item_t* key = cbor_build_string("x");
  cbor_item_t* val = cbor_map_get(map, key, cbor_structurally_equal);
  assert_non_null(val);
  assert_true(cbor_get_uint8(val) == 99);
  cbor_decref(&val);
  cbor_decref(&key);

  cbor_decref(&map);
}

static void test_get_negint_key(void** state _CBOR_UNUSED) {
  cbor_item_t* map = build_map();

  cbor_item_t* key = cbor_build_negint8(0);
  cbor_item_t* val = cbor_map_get(map, key, cbor_structurally_equal);
  assert_non_null(val);
  assert_true(cbor_isa_string(val));
  cbor_decref(&val);
  cbor_decref(&key);

  cbor_decref(&map);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_get_existing_key),
      cmocka_unit_test(test_get_missing_key),
      cmocka_unit_test(test_get_empty_map),
      cmocka_unit_test(test_get_wrong_type),
      cmocka_unit_test(test_get_encoding_width_matters_with_structural_eq),
      cmocka_unit_test(test_get_custom_eq_ignores_width),
      cmocka_unit_test(test_get_custom_str_eq),
      cmocka_unit_test(test_get_first_match_on_duplicate_keys),
      cmocka_unit_test(test_get_refcount),
      cmocka_unit_test(test_get_indefinite_map),
      cmocka_unit_test(test_get_negint_key),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
