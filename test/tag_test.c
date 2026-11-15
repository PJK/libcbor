/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#include <inttypes.h>
#include <stdio.h>

#include "assertions.h"
#include "cbor.h"
#include "test_allocator.h"

cbor_item_t* tag;
struct cbor_load_result res;

unsigned char embedded_tag_data[] = {0xC0, 0x00};

static void test_refcounting(void** _state _CBOR_UNUSED) {
  tag = cbor_load(embedded_tag_data, 2, &res);
  assert_true(cbor_refcount(tag) == 1);
  cbor_item_t* item = cbor_tag_item(tag);
  assert_true(cbor_refcount(item) == 2);
  cbor_decref(&tag);
  assert_null(tag);
  assert_true(cbor_refcount(item) == 1);
  cbor_decref(&item);
  assert_null(item);
}

/* Tag 0 + uint 0 */
static void test_embedded_tag(void** _state _CBOR_UNUSED) {
  tag = cbor_load(embedded_tag_data, 2, &res);
  assert_true(cbor_typeof(tag) == CBOR_TYPE_TAG);
  assert_true(cbor_tag_value(tag) == 0);
  assert_uint8(cbor_move(cbor_tag_item(tag)), 0);
  cbor_decref(&tag);
  assert_null(tag);
}

unsigned char int8_tag_data[] = {0xD8, 0xFF, 0x01};

/* Tag 255 + uint 1 */
static void test_int8_tag(void** _state _CBOR_UNUSED) {
  tag = cbor_load(int8_tag_data, 3, &res);
  assert_true(cbor_typeof(tag) == CBOR_TYPE_TAG);
  assert_true(cbor_tag_value(tag) == 255);
  assert_uint8(cbor_move(cbor_tag_item(tag)), 1);
  cbor_decref(&tag);
  assert_null(tag);
}

unsigned char int16_tag_data[] = {0xD9, 0xFF, 0x00, 0x02};

/* Tag 255 << 8 + uint 2 */
static void test_int16_tag(void** _state _CBOR_UNUSED) {
  tag = cbor_load(int16_tag_data, 4, &res);
  assert_true(cbor_typeof(tag) == CBOR_TYPE_TAG);
  assert_true(cbor_tag_value(tag) == 255 << 8);
  assert_uint8(cbor_move(cbor_tag_item(tag)), 2);
  cbor_decref(&tag);
  assert_null(tag);
}

unsigned char int32_tag_data[] = {0xDA, 0xFF, 0x00, 0x00, 0x00, 0x03};

/* uint 3 */
static void test_int32_tag(void** _state _CBOR_UNUSED) {
  tag = cbor_load(int32_tag_data, 6, &res);
  assert_true(cbor_typeof(tag) == CBOR_TYPE_TAG);
  assert_true(cbor_tag_value(tag) == 4278190080ULL);
  assert_uint8(cbor_move(cbor_tag_item(tag)), 3);
  cbor_decref(&tag);
  assert_null(tag);
}

unsigned char int64_tag_data[] = {0xDB, 0xFF, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x04};

/* uint 4 */
static void test_int64_tag(void** _state _CBOR_UNUSED) {
  tag = cbor_load(int64_tag_data, 10, &res);
  assert_true(cbor_typeof(tag) == CBOR_TYPE_TAG);
  assert_true(cbor_tag_value(tag) == 18374686479671623680ULL);
  assert_uint8(cbor_move(cbor_tag_item(tag)), 4);
  cbor_decref(&tag);
  assert_null(tag);
}

unsigned char nested_tag_data[] = {0xC0, 0xC1, 0x18, 0x2A};

/* Tag 0, tag 1 + uint 0 */
static void test_nested_tag(void** _state _CBOR_UNUSED) {
  tag = cbor_load(nested_tag_data, 4, &res);
  assert_true(cbor_typeof(tag) == CBOR_TYPE_TAG);
  assert_true(cbor_tag_value(tag) == 0);
  cbor_item_t* nested_tag = cbor_tag_item(tag);
  assert_true(cbor_typeof(nested_tag) == CBOR_TYPE_TAG);
  assert_true(cbor_tag_value(nested_tag) == 1);
  assert_uint8(cbor_move(cbor_tag_item(nested_tag)), 42);
  cbor_decref(&tag);
  assert_null(tag);
  cbor_decref(&nested_tag);
  assert_null(nested_tag);
}

static void test_all_tag_values_supported(void** _state _CBOR_UNUSED) {
  /* Test all items in the protected range of
   * https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml */
  for (uint64_t tag_value = 0; tag_value <= 32767; tag_value++) {
    cbor_item_t* tag_item =
        cbor_build_tag(tag_value, cbor_move(cbor_build_uint8(42)));
    unsigned char* serialized_tag;
    size_t serialized_tag_size =
        cbor_serialize_alloc(tag_item, &serialized_tag, NULL);
    assert_true(serialized_tag_size > 0);
    tag = cbor_load(serialized_tag, serialized_tag_size, &res);
    assert_true(res.read == serialized_tag_size);
    assert_true(cbor_typeof(tag) == CBOR_TYPE_TAG);
    assert_true(cbor_tag_value(tag) == tag_value);
    cbor_decref(&tag);
    assert_null(tag);
    cbor_decref(&tag_item);
    assert_null(tag_item);
    free(serialized_tag);
  }
}

static void test_build_tag(void** _state _CBOR_UNUSED) {
  tag = cbor_build_tag(1, cbor_move(cbor_build_uint8(42)));

  assert_true(cbor_typeof(tag) == CBOR_TYPE_TAG);
  assert_size_equal(cbor_tag_value(tag), 1);
  assert_uint8(cbor_move(cbor_tag_item(tag)), 42);

  cbor_decref(&tag);
}

static void test_build_tag_failure(void** _state _CBOR_UNUSED) {
  cbor_item_t* tagged_item = cbor_build_uint8(42);

  WITH_FAILING_MALLOC({ assert_null(cbor_build_tag(1, tagged_item)); });
  assert_size_equal(cbor_refcount(tagged_item), 1);

  cbor_decref(&tagged_item);
}

static void test_set_item_replaces_previous(void** _state _CBOR_UNUSED) {
  cbor_item_t* item1 = cbor_build_uint8(1);
  cbor_item_t* item2 = cbor_build_uint8(2);
  tag = cbor_build_tag(0, item1);

  assert_size_equal(cbor_refcount(item1), 2);
  assert_size_equal(cbor_refcount(item2), 1);

  cbor_tag_set_item(tag, item2);

  // item1 should have been released by cbor_tag_set_item
  assert_size_equal(cbor_refcount(item1), 1);
  assert_size_equal(cbor_refcount(item2), 2);
  assert_uint8(cbor_move(cbor_tag_item(tag)), 2);

  cbor_decref(&item1);
  cbor_decref(&item2);
  cbor_decref(&tag);
}

static void test_tag_item_null(void** _state _CBOR_UNUSED) {
  tag = cbor_new_tag(0);
  assert_null(cbor_tag_item(tag));
  cbor_decref(&tag);
}

static void test_move_null(void** _state _CBOR_UNUSED) {
  // cbor_tag_item returns NULL on a tag with no item set; cbor_move must
  // handle NULL without crashing
  tag = cbor_new_tag(0);
  assert_null(cbor_move(cbor_tag_item(tag)));
  cbor_decref(&tag);
}

static void test_tag_creation(void** _state _CBOR_UNUSED) {
  WITH_FAILING_MALLOC({ assert_null(cbor_new_tag(42)); });
}

#ifdef LIBCBOR_TAGS_H
#if CBOR_TAG_DATETIME_STRING != 0 || CBOR_TAG_SELF_DESCRIBED_CBOR != 55799
#error "Unexpected CBOR_TAG_* value in preprocessor expression"
#endif
#endif

static void test_known_tags(void** _state _CBOR_UNUSED) {
  assert_size_equal(sizeof(CBOR_TAG_DATETIME_STRING), sizeof(uint64_t));
  assert_size_equal(sizeof(CBOR_TAG_SELF_DESCRIBED_CBOR), sizeof(uint64_t));

  char formatted[32];
  assert_true(snprintf(formatted, sizeof(formatted), "%" PRIu64,
                       CBOR_TAG_SELF_DESCRIBED_CBOR) > 0);
  assert_string_equal(formatted, "55799");

  struct known_tag_case {
    uint64_t tag_value;
    unsigned char expected_wire[4];
    size_t expected_wire_len;
  };
  /* Each case tags uint8(0) (encoded as 0x00) with an RFC 8949 Table 5 tag. */
  const struct known_tag_case cases[] = {
      {CBOR_TAG_DATETIME_STRING, {0xC0, 0x00}, 2},
      {CBOR_TAG_DATETIME_EPOCH, {0xC1, 0x00}, 2},
      {CBOR_TAG_UNSIGNED_BIGNUM, {0xC2, 0x00}, 2},
      {CBOR_TAG_NEGATIVE_BIGNUM, {0xC3, 0x00}, 2},
      {CBOR_TAG_DECIMAL_FRACTION, {0xC4, 0x00}, 2},
      {CBOR_TAG_BIGFLOAT, {0xC5, 0x00}, 2},
      {CBOR_TAG_EXPECTED_BASE64URL, {0xD5, 0x00}, 2},
      {CBOR_TAG_EXPECTED_BASE64, {0xD6, 0x00}, 2},
      {CBOR_TAG_EXPECTED_BASE16, {0xD7, 0x00}, 2},
      {CBOR_TAG_ENCODED_CBOR, {0xD8, 0x18, 0x00}, 3},
      {CBOR_TAG_URI, {0xD8, 0x20, 0x00}, 3},
      {CBOR_TAG_BASE64URL, {0xD8, 0x21, 0x00}, 3},
      {CBOR_TAG_BASE64, {0xD8, 0x22, 0x00}, 3},
      {CBOR_TAG_MIME_MESSAGE, {0xD8, 0x24, 0x00}, 3},
      {CBOR_TAG_SELF_DESCRIBED_CBOR, {0xD9, 0xD9, 0xF7, 0x00}, 4},
  };

  size_t matched_in_switch = 0;
  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
    cbor_item_t* built =
        cbor_build_tag(cases[i].tag_value, cbor_move(cbor_build_uint8(0)));
    assert_non_null(built);

    unsigned char encoded[8];
    size_t encoded_len = cbor_serialize(built, encoded, sizeof(encoded));
    assert_size_equal(encoded_len, cases[i].expected_wire_len);
    assert_memory_equal(encoded, cases[i].expected_wire,
                        cases[i].expected_wire_len);

    cbor_item_t* loaded = cbor_load(encoded, encoded_len, &res);
    assert_non_null(loaded);
    assert_int_equal(res.error.code, CBOR_ERR_NONE);
    assert_size_equal(res.read, encoded_len);
    assert_true(cbor_isa_tag(loaded));
    assert_true(cbor_tag_value(loaded) == cases[i].tag_value);

    switch (cbor_tag_value(loaded)) {
      case CBOR_TAG_DATETIME_STRING:
      case CBOR_TAG_DATETIME_EPOCH:
      case CBOR_TAG_UNSIGNED_BIGNUM:
      case CBOR_TAG_NEGATIVE_BIGNUM:
      case CBOR_TAG_DECIMAL_FRACTION:
      case CBOR_TAG_BIGFLOAT:
      case CBOR_TAG_EXPECTED_BASE64URL:
      case CBOR_TAG_EXPECTED_BASE64:
      case CBOR_TAG_EXPECTED_BASE16:
      case CBOR_TAG_ENCODED_CBOR:
      case CBOR_TAG_URI:
      case CBOR_TAG_BASE64URL:
      case CBOR_TAG_BASE64:
      case CBOR_TAG_MIME_MESSAGE:
      case CBOR_TAG_SELF_DESCRIBED_CBOR:
        matched_in_switch++;
        break;
      default:
        fail();
    }

    cbor_decref(&loaded);
    cbor_decref(&built);
  }
  assert_size_equal(matched_in_switch, sizeof(cases) / sizeof(cases[0]));
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_refcounting),
      cmocka_unit_test(test_embedded_tag),
      cmocka_unit_test(test_int8_tag),
      cmocka_unit_test(test_int16_tag),
      cmocka_unit_test(test_int32_tag),
      cmocka_unit_test(test_int64_tag),
      cmocka_unit_test(test_nested_tag),
      cmocka_unit_test(test_all_tag_values_supported),
      cmocka_unit_test(test_build_tag),
      cmocka_unit_test(test_build_tag_failure),
      cmocka_unit_test(test_set_item_replaces_previous),
      cmocka_unit_test(test_tag_item_null),
      cmocka_unit_test(test_move_null),
      cmocka_unit_test(test_tag_creation),
      cmocka_unit_test(test_known_tags),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
