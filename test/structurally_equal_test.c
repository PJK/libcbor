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

#include "assertions.h"
#include "cbor.h"

/* -------------------------------------------------------------------------
 * Integers
 * ---------------------------------------------------------------------- */

static void test_uint_equal(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_uint8(10);
  cbor_item_t* b = cbor_build_uint8(10);
  assert_true(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_uint_different_value(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_uint8(10);
  cbor_item_t* b = cbor_build_uint8(11);
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_uint_different_width(void** state _CBOR_UNUSED) {
  /* uint8(1) and uint16(1) represent the same integer value but are
   * structurally distinct because the encoding width differs. */
  cbor_item_t* a = cbor_build_uint8(1);
  cbor_item_t* b = cbor_build_uint16(1);
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);

  cbor_item_t* c = cbor_build_uint16(1);
  cbor_item_t* d = cbor_build_uint32(1);
  assert_false(cbor_structurally_equal(c, d));
  cbor_decref(&c);
  cbor_decref(&d);

  cbor_item_t* e = cbor_build_uint32(1);
  cbor_item_t* f = cbor_build_uint64(1);
  assert_false(cbor_structurally_equal(e, f));
  cbor_decref(&e);
  cbor_decref(&f);
}

static void test_negint_equal(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_negint8(5);
  cbor_item_t* b = cbor_build_negint8(5);
  assert_true(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_negint_different_width(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_negint8(1);
  cbor_item_t* b = cbor_build_negint16(1);
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_uint_vs_negint(void** state _CBOR_UNUSED) {
  /* Major type distinguishes UINT from NEGINT. */
  cbor_item_t* a = cbor_build_uint8(0);
  cbor_item_t* b = cbor_build_negint8(0);
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

/* -------------------------------------------------------------------------
 * Bytestrings
 * ---------------------------------------------------------------------- */

static void test_bytestring_equal(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_bytestring((cbor_mutable_data) "hello", 5);
  cbor_item_t* b = cbor_build_bytestring((cbor_mutable_data) "hello", 5);
  assert_true(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_bytestring_different_content(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_bytestring((cbor_mutable_data) "hello", 5);
  cbor_item_t* b = cbor_build_bytestring((cbor_mutable_data) "world", 5);
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_bytestring_different_length(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_bytestring((cbor_mutable_data) "hello", 5);
  cbor_item_t* b = cbor_build_bytestring((cbor_mutable_data) "hello", 4);
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_bytestring_definite_vs_indefinite(void** state _CBOR_UNUSED) {
  /* A definite and an indefinite bytestring with the same bytes are
   * structurally distinct. */
  cbor_item_t* definite = cbor_build_bytestring((cbor_mutable_data) "hello", 5);

  cbor_item_t* indefinite = cbor_new_indefinite_bytestring();
  cbor_item_t* chunk = cbor_build_bytestring((cbor_mutable_data) "hello", 5);
  assert_true(cbor_bytestring_add_chunk(indefinite, cbor_move(chunk)));

  assert_false(cbor_structurally_equal(definite, indefinite));
  cbor_decref(&definite);
  cbor_decref(&indefinite);
}

static void test_bytestring_indefinite_chunk_structure(
    void** state _CBOR_UNUSED) {
  /* Two indefinite bytestrings with the same bytes but different chunk
   * boundaries are structurally distinct. */
  cbor_item_t* a = cbor_new_indefinite_bytestring();
  cbor_item_t* a1 = cbor_build_bytestring((cbor_mutable_data) "he", 2);
  cbor_item_t* a2 = cbor_build_bytestring((cbor_mutable_data) "llo", 3);
  assert_true(cbor_bytestring_add_chunk(a, cbor_move(a1)));
  assert_true(cbor_bytestring_add_chunk(a, cbor_move(a2)));

  cbor_item_t* b = cbor_new_indefinite_bytestring();
  cbor_item_t* b1 = cbor_build_bytestring((cbor_mutable_data) "hello", 5);
  assert_true(cbor_bytestring_add_chunk(b, cbor_move(b1)));

  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_bytestring_indefinite_equal(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_new_indefinite_bytestring();
  cbor_item_t* a1 = cbor_build_bytestring((cbor_mutable_data) "he", 2);
  cbor_item_t* a2 = cbor_build_bytestring((cbor_mutable_data) "llo", 3);
  assert_true(cbor_bytestring_add_chunk(a, cbor_move(a1)));
  assert_true(cbor_bytestring_add_chunk(a, cbor_move(a2)));

  cbor_item_t* b = cbor_new_indefinite_bytestring();
  cbor_item_t* b1 = cbor_build_bytestring((cbor_mutable_data) "he", 2);
  cbor_item_t* b2 = cbor_build_bytestring((cbor_mutable_data) "llo", 3);
  assert_true(cbor_bytestring_add_chunk(b, cbor_move(b1)));
  assert_true(cbor_bytestring_add_chunk(b, cbor_move(b2)));

  assert_true(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

/* -------------------------------------------------------------------------
 * Strings
 * ---------------------------------------------------------------------- */

static void test_string_equal(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_string("hello");
  cbor_item_t* b = cbor_build_string("hello");
  assert_true(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_string_different(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_string("hello");
  cbor_item_t* b = cbor_build_string("world");
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_string_vs_bytestring(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_string("hi");
  cbor_item_t* b = cbor_build_bytestring((cbor_mutable_data) "hi", 2);
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_string_definite_vs_indefinite(void** state _CBOR_UNUSED) {
  cbor_item_t* definite = cbor_build_string("hello");

  cbor_item_t* indefinite = cbor_new_indefinite_string();
  cbor_item_t* chunk = cbor_build_string("hello");
  assert_true(cbor_string_add_chunk(indefinite, cbor_move(chunk)));

  assert_false(cbor_structurally_equal(definite, indefinite));
  cbor_decref(&definite);
  cbor_decref(&indefinite);
}

/* -------------------------------------------------------------------------
 * Arrays
 * ---------------------------------------------------------------------- */

static void test_array_equal(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_new_definite_array(2);
  assert_true(cbor_array_push(a, cbor_move(cbor_build_uint8(1))));
  assert_true(cbor_array_push(a, cbor_move(cbor_build_uint8(2))));

  cbor_item_t* b = cbor_new_definite_array(2);
  assert_true(cbor_array_push(b, cbor_move(cbor_build_uint8(1))));
  assert_true(cbor_array_push(b, cbor_move(cbor_build_uint8(2))));

  assert_true(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_array_different_order(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_new_definite_array(2);
  assert_true(cbor_array_push(a, cbor_move(cbor_build_uint8(1))));
  assert_true(cbor_array_push(a, cbor_move(cbor_build_uint8(2))));

  cbor_item_t* b = cbor_new_definite_array(2);
  assert_true(cbor_array_push(b, cbor_move(cbor_build_uint8(2))));
  assert_true(cbor_array_push(b, cbor_move(cbor_build_uint8(1))));

  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_array_definite_vs_indefinite(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_new_definite_array(1);
  assert_true(cbor_array_push(a, cbor_move(cbor_build_uint8(42))));

  cbor_item_t* b = cbor_new_indefinite_array();
  assert_true(cbor_array_push(b, cbor_move(cbor_build_uint8(42))));

  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_array_element_width_matters(void** state _CBOR_UNUSED) {
  /* Elements' encoding widths are part of the structure. */
  cbor_item_t* a = cbor_new_definite_array(1);
  assert_true(cbor_array_push(a, cbor_move(cbor_build_uint8(1))));

  cbor_item_t* b = cbor_new_definite_array(1);
  assert_true(cbor_array_push(b, cbor_move(cbor_build_uint16(1))));

  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

/* -------------------------------------------------------------------------
 * Maps
 * ---------------------------------------------------------------------- */

static void test_map_equal(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_new_definite_map(2);
  assert_true(cbor_map_add(
      a, (struct cbor_pair){.key = cbor_move(cbor_build_string("x")),
                            .value = cbor_move(cbor_build_uint8(1))}));
  assert_true(cbor_map_add(
      a, (struct cbor_pair){.key = cbor_move(cbor_build_string("y")),
                            .value = cbor_move(cbor_build_uint8(2))}));

  cbor_item_t* b = cbor_new_definite_map(2);
  assert_true(cbor_map_add(
      b, (struct cbor_pair){.key = cbor_move(cbor_build_string("x")),
                            .value = cbor_move(cbor_build_uint8(1))}));
  assert_true(cbor_map_add(
      b, (struct cbor_pair){.key = cbor_move(cbor_build_string("y")),
                            .value = cbor_move(cbor_build_uint8(2))}));

  assert_true(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_map_different_order(void** state _CBOR_UNUSED) {
  /* Structural equality is positional: the same key-value pairs in a different
   * encoded order are not structurally equal. */
  cbor_item_t* a = cbor_new_definite_map(2);
  assert_true(cbor_map_add(
      a, (struct cbor_pair){.key = cbor_move(cbor_build_string("x")),
                            .value = cbor_move(cbor_build_uint8(1))}));
  assert_true(cbor_map_add(
      a, (struct cbor_pair){.key = cbor_move(cbor_build_string("y")),
                            .value = cbor_move(cbor_build_uint8(2))}));

  cbor_item_t* b = cbor_new_definite_map(2);
  assert_true(cbor_map_add(
      b, (struct cbor_pair){.key = cbor_move(cbor_build_string("y")),
                            .value = cbor_move(cbor_build_uint8(2))}));
  assert_true(cbor_map_add(
      b, (struct cbor_pair){.key = cbor_move(cbor_build_string("x")),
                            .value = cbor_move(cbor_build_uint8(1))}));

  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_map_key_width_matters(void** state _CBOR_UNUSED) {
  /* uint8(1) and uint16(1) are structurally distinct keys. */
  cbor_item_t* a = cbor_new_definite_map(1);
  assert_true(cbor_map_add(
      a, (struct cbor_pair){.key = cbor_move(cbor_build_uint8(1)),
                            .value = cbor_move(cbor_build_uint8(0))}));

  cbor_item_t* b = cbor_new_definite_map(1);
  assert_true(cbor_map_add(
      b, (struct cbor_pair){.key = cbor_move(cbor_build_uint16(1)),
                            .value = cbor_move(cbor_build_uint8(0))}));

  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_map_definite_vs_indefinite(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_new_definite_map(1);
  assert_true(cbor_map_add(
      a, (struct cbor_pair){.key = cbor_move(cbor_build_uint8(0)),
                            .value = cbor_move(cbor_build_uint8(0))}));

  cbor_item_t* b = cbor_new_indefinite_map();
  assert_true(cbor_map_add(
      b, (struct cbor_pair){.key = cbor_move(cbor_build_uint8(0)),
                            .value = cbor_move(cbor_build_uint8(0))}));

  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

/* -------------------------------------------------------------------------
 * Tags
 * ---------------------------------------------------------------------- */

static void test_tag_equal(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_tag(1, cbor_move(cbor_build_uint32(1000000)));
  cbor_item_t* b = cbor_build_tag(1, cbor_move(cbor_build_uint32(1000000)));
  assert_true(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_tag_different_number(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_tag(1, cbor_move(cbor_build_uint32(0)));
  cbor_item_t* b = cbor_build_tag(2, cbor_move(cbor_build_uint32(0)));
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_tag_different_content_width(void** state _CBOR_UNUSED) {
  /* The tagged item's encoding width is part of the structure. */
  cbor_item_t* a = cbor_build_tag(1, cbor_move(cbor_build_uint8(42)));
  cbor_item_t* b = cbor_build_tag(1, cbor_move(cbor_build_uint16(42)));
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

/* -------------------------------------------------------------------------
 * Floats and control values
 * ---------------------------------------------------------------------- */

static void test_float_equal(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_float4(1.5f);
  cbor_item_t* b = cbor_build_float4(1.5f);
  assert_true(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_float_different_width(void** state _CBOR_UNUSED) {
  /* float2(1.5) and float4(1.5) encode the same numeric value but use
   * different encoding widths, so they are not structurally equal. */
  cbor_item_t* a = cbor_build_float2(1.5f);
  cbor_item_t* b = cbor_build_float4(1.5f);
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);

  cbor_item_t* c = cbor_build_float4(1.5f);
  cbor_item_t* d = cbor_build_float8(1.5);
  assert_false(cbor_structurally_equal(c, d));
  cbor_decref(&c);
  cbor_decref(&d);
}

static void test_bool_equal(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_bool(true);
  cbor_item_t* b = cbor_build_bool(true);
  assert_true(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_bool_different(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_bool(true);
  cbor_item_t* b = cbor_build_bool(false);
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_null_equal(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_new_null();
  cbor_item_t* b = cbor_new_null();
  assert_true(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_null_vs_undef(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_new_null();
  cbor_item_t* b = cbor_new_undef();
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_float_vs_int(void** state _CBOR_UNUSED) {
  /* In the CBOR data model, integers and floats are distinct types. */
  cbor_item_t* a = cbor_build_uint8(1);
  cbor_item_t* b = cbor_build_float4(1.0f);
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

/* -------------------------------------------------------------------------
 * Reflexivity spot-checks
 * ---------------------------------------------------------------------- */

static void test_reflexivity(void** state _CBOR_UNUSED) {
  cbor_item_t* items[] = {
      cbor_build_uint64(UINT64_MAX),
      cbor_build_negint32(12345),
      cbor_build_string("reflexive"),
      cbor_build_bytestring((cbor_mutable_data) "\x00\xff", 2),
      cbor_build_bool(false),
      cbor_new_null(),
      cbor_build_float8(3.14159265358979323846),
  };
  for (size_t i = 0; i < sizeof(items) / sizeof(items[0]); i++) {
    assert_true(cbor_structurally_equal(items[i], items[i]));
    cbor_decref(&items[i]);
  }
}

/* -------------------------------------------------------------------------
 * Cross-type spot-checks
 * ---------------------------------------------------------------------- */

static void test_cross_type(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_build_uint8(0);
  cbor_item_t* b = cbor_build_string("0");
  cbor_item_t* c = cbor_new_null();
  cbor_item_t* d = cbor_build_bool(false);

  assert_false(cbor_structurally_equal(a, b));
  assert_false(cbor_structurally_equal(a, c));
  assert_false(cbor_structurally_equal(c, d));

  cbor_decref(&a);
  cbor_decref(&b);
  cbor_decref(&c);
  cbor_decref(&d);
}

/* -------------------------------------------------------------------------
 * Additional coverage tests
 * These target specific branches that the cases above do not reach.
 * ---------------------------------------------------------------------- */

static void test_bytestring_indefinite_unequal_chunks(
    void** state _CBOR_UNUSED) {
  /* Same chunk count, different content: exercises the return-false path
   * inside the indefinite bytestring chunk loop. */
  cbor_item_t* a = cbor_new_indefinite_bytestring();
  cbor_item_t* b = cbor_new_indefinite_bytestring();
  assert_true(cbor_bytestring_add_chunk(
      a, cbor_move(cbor_build_bytestring((cbor_mutable_data) "ab", 2))));
  assert_true(cbor_bytestring_add_chunk(
      b, cbor_move(cbor_build_bytestring((cbor_mutable_data) "xy", 2))));
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_string_indefinite_unequal_chunks(void** state _CBOR_UNUSED) {
  /* Same chunk count, different content: exercises the return-false path
   * inside the indefinite string chunk loop. */
  cbor_item_t* a = cbor_new_indefinite_string();
  cbor_item_t* b = cbor_new_indefinite_string();
  assert_true(cbor_string_add_chunk(a, cbor_move(cbor_build_string("hello"))));
  assert_true(cbor_string_add_chunk(b, cbor_move(cbor_build_string("world"))));
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_array_different_size(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_new_definite_array(1);
  assert_true(cbor_array_push(a, cbor_move(cbor_build_uint8(1))));
  cbor_item_t* b = cbor_new_definite_array(2);
  assert_true(cbor_array_push(b, cbor_move(cbor_build_uint8(1))));
  assert_true(cbor_array_push(b, cbor_move(cbor_build_uint8(2))));
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_map_different_size(void** state _CBOR_UNUSED) {
  cbor_item_t* a = cbor_new_definite_map(1);
  assert_true(cbor_map_add(
      a, (struct cbor_pair){.key = cbor_move(cbor_build_uint8(0)),
                            .value = cbor_move(cbor_build_uint8(0))}));
  cbor_item_t* b = cbor_new_definite_map(2);
  assert_true(cbor_map_add(
      b, (struct cbor_pair){.key = cbor_move(cbor_build_uint8(0)),
                            .value = cbor_move(cbor_build_uint8(0))}));
  assert_true(cbor_map_add(
      b, (struct cbor_pair){.key = cbor_move(cbor_build_uint8(1)),
                            .value = cbor_move(cbor_build_uint8(1))}));
  assert_false(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_tag_both_null_item(void** state _CBOR_UNUSED) {
  /* Tags whose tagged item has not been set yet: both NULL → equal. */
  cbor_item_t* a = cbor_new_tag(1);
  cbor_item_t* b = cbor_new_tag(1);
  assert_true(cbor_structurally_equal(a, b));
  cbor_decref(&a);
  cbor_decref(&b);
}

static void test_tag_one_null_item(void** state _CBOR_UNUSED) {
  /* One tag has a tagged item, the other does not → not equal. */
  cbor_item_t* a = cbor_new_tag(1);
  cbor_item_t* b = cbor_build_tag(1, cbor_move(cbor_build_uint8(0)));
  assert_false(cbor_structurally_equal(a, b));
  assert_false(cbor_structurally_equal(b, a));
  cbor_decref(&a);
  cbor_decref(&b);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      /* Integers */
      cmocka_unit_test(test_uint_equal),
      cmocka_unit_test(test_uint_different_value),
      cmocka_unit_test(test_uint_different_width),
      cmocka_unit_test(test_negint_equal),
      cmocka_unit_test(test_negint_different_width),
      cmocka_unit_test(test_uint_vs_negint),
      /* Bytestrings */
      cmocka_unit_test(test_bytestring_equal),
      cmocka_unit_test(test_bytestring_different_content),
      cmocka_unit_test(test_bytestring_different_length),
      cmocka_unit_test(test_bytestring_definite_vs_indefinite),
      cmocka_unit_test(test_bytestring_indefinite_chunk_structure),
      cmocka_unit_test(test_bytestring_indefinite_equal),
      /* Strings */
      cmocka_unit_test(test_string_equal),
      cmocka_unit_test(test_string_different),
      cmocka_unit_test(test_string_vs_bytestring),
      cmocka_unit_test(test_string_definite_vs_indefinite),
      /* Arrays */
      cmocka_unit_test(test_array_equal),
      cmocka_unit_test(test_array_different_order),
      cmocka_unit_test(test_array_definite_vs_indefinite),
      cmocka_unit_test(test_array_element_width_matters),
      /* Maps */
      cmocka_unit_test(test_map_equal),
      cmocka_unit_test(test_map_different_order),
      cmocka_unit_test(test_map_key_width_matters),
      cmocka_unit_test(test_map_definite_vs_indefinite),
      /* Tags */
      cmocka_unit_test(test_tag_equal),
      cmocka_unit_test(test_tag_different_number),
      cmocka_unit_test(test_tag_different_content_width),
      /* Floats and ctrl */
      cmocka_unit_test(test_float_equal),
      cmocka_unit_test(test_float_different_width),
      cmocka_unit_test(test_bool_equal),
      cmocka_unit_test(test_bool_different),
      cmocka_unit_test(test_null_equal),
      cmocka_unit_test(test_null_vs_undef),
      cmocka_unit_test(test_float_vs_int),
      /* General */
      cmocka_unit_test(test_reflexivity),
      cmocka_unit_test(test_cross_type),
      /* Coverage */
      cmocka_unit_test(test_bytestring_indefinite_unequal_chunks),
      cmocka_unit_test(test_string_indefinite_unequal_chunks),
      cmocka_unit_test(test_array_different_size),
      cmocka_unit_test(test_map_different_size),
      cmocka_unit_test(test_tag_both_null_item),
      cmocka_unit_test(test_tag_one_null_item),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
