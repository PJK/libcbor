/*
 * Copyright (c) 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

/* Runs the CBOR Working Group test vectors
 * (https://github.com/cbor-wg/cbor-test-vectors) vendored in
 * test/data/cbor-test-vectors. See the README there for the container format.
 *
 * For every vector, `encoded` is decoded with cbor_load. Vectors marked
 * `fail` must be rejected. All others must decode fully, compare equal to
 * `decoded` by value (see value_equal), and, unless `roundtrip` is false,
 * serialize back to exactly `encoded`. Good vectors are also fed through
 * cbor_stream_decode to check that the streaming decoder consumes them
 * without error.
 *
 * Known deviations are listed in expected_failures below. An expected
 * failure that starts passing fails the test so that the list is kept
 * current. */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assertions.h"
#include "cbor.h"

#ifndef CBOR_TEST_VECTORS_DIR
#error "CBOR_TEST_VECTORS_DIR must point at test/data/cbor-test-vectors"
#endif

struct expected_failure {
  const char* file;         // Suite file, relative to CBOR_TEST_VECTORS_DIR
  const char* description;  // Vector description, exact match
  const char* encoded;      // Lowercase hex of `encoded`, exact match
  const char* reason;
};

/* Keep this list sorted by file, then by encoded */
static const struct expected_failure expected_failures[] = {
    {"rfc8949/bad.cbor", "utf8: invalid utf8", "62c0ae",
     "Invalid UTF-8 is a validity error (RFC 8949 5.3.1), not a "
     "well-formedness error; libcbor decoders check well-formedness only"},
    {"rfc8949/bad.cbor", "date: unexpected object instead of string",
     "c0a1616100",
     "Tag content type is a validity error (RFC 8949 5.3.2); libcbor does "
     "not understand tag semantics"},
    {"rfc8949/bad.cbor", "date: unexpected object instead of offset",
     "c1a1616100",
     "Tag content type is a validity error (RFC 8949 5.3.2); libcbor does "
     "not understand tag semantics"},
    // Single- and double-precision NaN payloads are canonicalized on encoding
    // (see cbor_encode_single, cbor_encode_double), so the round trip does not
    // preserve them
    {"spike/spike.cbor", "DLO/PS/CDE/LDE", "fa7fa3f553", "NaN payload"},
    {"spike/spike.cbor", "DLO/PS/CDE/LDE", "fa7fa86197", "NaN payload"},
    {"spike/spike.cbor", "DLO/PS/CDE/LDE", "fa7fbec01b", "NaN payload"},
    {"spike/spike.cbor", "DLO/PS/CDE/LDE", "faffbd3eb2", "NaN payload"},
    {"spike/spike.cbor", "DLO/PS/CDE/LDE", "faffca24fe", "NaN payload"},
    {"spike/spike.cbor", "DLO/PS/CDE/LDE", "faffddb719", "NaN payload"},
    {"spike/spike.cbor", "DLO/PS/CDE/LDE", "fb7ff47eaa6bb744df", "NaN payload"},
    {"spike/spike.cbor", "DLO/PS/CDE/LDE", "fb7ff50c32fdc0b06d", "NaN payload"},
    {"spike/spike.cbor", "DLO/PS/CDE/LDE", "fb7ff7d8037701b83c", "NaN payload"},
    {"spike/spike.cbor", "DLO/PS/CDE/LDE", "fbfff7a7d642e1b3ff", "NaN payload"},
    {"spike/spike.cbor", "DLO/PS/CDE/LDE", "fbfff9449fd767f03e", "NaN payload"},
    {"spike/spike.cbor", "DLO/PS/CDE/LDE", "fbfffbb6e3314b47ad", "NaN payload"},
};

/* Per-test bookkeeping */
static int vectors_run;
static int unexpected_failures;
static int unexpected_passes;
static int expected_failures_seen;

static void reset_counters(void) {
  vectors_run = 0;
  unexpected_failures = 0;
  unexpected_passes = 0;
  expected_failures_seen = 0;
}

static unsigned char* read_file(const char* relative_path, size_t* length) {
  char path[1024];
  snprintf(path, sizeof(path), "%s/%s", CBOR_TEST_VECTORS_DIR, relative_path);
  FILE* f = fopen(path, "rb");
  if (f == NULL) {
    // Not using `fail_msg`, see _assert_size_equal
    print_error("Cannot open %s\n", path);
    fail();
  }
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  rewind(f);
  assert_true(size > 0);
  unsigned char* data = malloc((size_t)size);
  assert_non_null(data);
  assert_size_equal(fread(data, 1, (size_t)size, f), (size_t)size);
  fclose(f);
  *length = (size_t)size;
  return data;
}

static char* to_hex(cbor_data data, size_t length) {
  char* hex = malloc(length * 2 + 1);
  assert_non_null(hex);
  for (size_t i = 0; i < length; i++) {
    snprintf(hex + 2 * i, 3, "%02x", data[i]);
  }
  hex[length * 2] = '\0';
  return hex;
}

/* Container access */

static cbor_item_t* map_get(cbor_item_t* map, const char* key) {
  assert_true(cbor_isa_map(map));
  struct cbor_pair* pairs = cbor_map_handle(map);
  size_t key_length = strlen(key);
  for (size_t i = 0; i < cbor_map_size(map); i++) {
    if (cbor_isa_string(pairs[i].key) &&
        cbor_string_is_definite(pairs[i].key) &&
        cbor_string_length(pairs[i].key) == key_length &&
        memcmp(cbor_string_handle(pairs[i].key), key, key_length) == 0) {
      return pairs[i].value;
    }
  }
  return NULL;
}

static bool get_bool(cbor_item_t* map, const char* key, bool fallback) {
  cbor_item_t* value = map_get(map, key);
  if (value == NULL) return fallback;
  assert_true(cbor_is_bool(value));
  return cbor_get_bool(value);
}

/* Value equality
 *
 * The `decoded` items are produced by another encoder in preferred form, so
 * the comparison must ignore encoding details that libcbor preserves:
 * integer and float widths, string definiteness and chunking, and the
 * representation of bignums (tag 2/3 versus plain integers). */

/* Concatenated content of a (possibly indefinite) string or byte string */
static unsigned char* flatten(cbor_item_t* item, size_t* length) {
  bool is_string = cbor_isa_string(item);
  bool is_definite = is_string ? cbor_string_is_definite(item)
                               : cbor_bytestring_is_definite(item);
  if (is_definite) {
    *length =
        is_string ? cbor_string_length(item) : cbor_bytestring_length(item);
    unsigned char* data = malloc(*length + 1);
    assert_non_null(data);
    if (*length > 0) {
      memcpy(
          data,
          is_string ? cbor_string_handle(item) : cbor_bytestring_handle(item),
          *length);
    }
    return data;
  }

  size_t chunk_count = is_string ? cbor_string_chunk_count(item)
                                 : cbor_bytestring_chunk_count(item);
  cbor_item_t** chunks = is_string ? cbor_string_chunks_handle(item)
                                   : cbor_bytestring_chunks_handle(item);
  *length = 0;
  unsigned char* data = malloc(1);
  assert_non_null(data);
  for (size_t i = 0; i < chunk_count; i++) {
    size_t chunk_length;
    unsigned char* chunk_data = flatten(chunks[i], &chunk_length);
    data = realloc(data, *length + chunk_length + 1);
    assert_non_null(data);
    memcpy(data + *length, chunk_data, chunk_length);
    *length += chunk_length;
    free(chunk_data);
  }
  return data;
}

/* Big-endian magnitude of an integer or a tag 2/3 bignum with leading zeros
 * stripped. Returns false if the item is neither. */
static bool integer_magnitude(cbor_item_t* item, unsigned char** magnitude,
                              size_t* length, bool* negative) {
  if (cbor_isa_uint(item) || cbor_isa_negint(item)) {
    uint64_t value = cbor_get_int(item);
    *negative = cbor_isa_negint(item);
    *magnitude = malloc(8);
    assert_non_null(*magnitude);
    *length = 0;
    for (int i = 7; i >= 0; i--) {
      unsigned char byte = (unsigned char)(value >> (8 * i));
      if (*length > 0 || byte != 0) (*magnitude)[(*length)++] = byte;
    }
    return true;
  }
  if (cbor_isa_tag(item) &&
      (cbor_tag_value(item) == 2 || cbor_tag_value(item) == 3)) {
    // cbor_tag_item returns a new reference
    cbor_item_t* content = cbor_tag_item(item);
    if (!cbor_isa_bytestring(content)) {
      cbor_decref(&content);
      return false;
    }
    size_t raw_length;
    unsigned char* raw = flatten(content, &raw_length);
    cbor_decref(&content);
    size_t skip = 0;
    while (skip < raw_length && raw[skip] == 0) skip++;
    *length = raw_length - skip;
    *magnitude = malloc(*length + 1);
    assert_non_null(*magnitude);
    memcpy(*magnitude, raw + skip, *length);
    free(raw);
    *negative = cbor_tag_value(item) == 3;
    return true;
  }
  return false;
}

static bool value_equal(cbor_item_t* a, cbor_item_t* b) {
  unsigned char *magnitude_a, *magnitude_b;
  size_t length_a, length_b;
  bool negative_a, negative_b;
  if (integer_magnitude(a, &magnitude_a, &length_a, &negative_a)) {
    if (!integer_magnitude(b, &magnitude_b, &length_b, &negative_b)) {
      free(magnitude_a);
      return false;
    }
    bool equal = negative_a == negative_b && length_a == length_b &&
                 memcmp(magnitude_a, magnitude_b, length_a) == 0;
    free(magnitude_a);
    free(magnitude_b);
    return equal;
  }

  if (cbor_typeof(a) != cbor_typeof(b)) return false;

  switch (cbor_typeof(a)) {
    case CBOR_TYPE_UINT:
    case CBOR_TYPE_NEGINT:
      // Handled above
      return false;  // LCOV_EXCL_LINE
    case CBOR_TYPE_BYTESTRING:
    case CBOR_TYPE_STRING: {
      size_t length_a, length_b;
      unsigned char* data_a = flatten(a, &length_a);
      unsigned char* data_b = flatten(b, &length_b);
      bool equal = length_a == length_b &&
                   (length_a == 0 || memcmp(data_a, data_b, length_a) == 0);
      free(data_a);
      free(data_b);
      return equal;
    }
    case CBOR_TYPE_ARRAY: {
      if (cbor_array_size(a) != cbor_array_size(b)) return false;
      for (size_t i = 0; i < cbor_array_size(a); i++) {
        if (!value_equal(cbor_array_handle(a)[i], cbor_array_handle(b)[i]))
          return false;
      }
      return true;
    }
    case CBOR_TYPE_MAP: {
      if (cbor_map_size(a) != cbor_map_size(b)) return false;
      for (size_t i = 0; i < cbor_map_size(a); i++) {
        if (!value_equal(cbor_map_handle(a)[i].key,
                         cbor_map_handle(b)[i].key) ||
            !value_equal(cbor_map_handle(a)[i].value,
                         cbor_map_handle(b)[i].value))
          return false;
      }
      return true;
    }
    case CBOR_TYPE_TAG: {
      if (cbor_tag_value(a) != cbor_tag_value(b)) return false;
      // cbor_tag_item returns a new reference
      cbor_item_t* content_a = cbor_tag_item(a);
      cbor_item_t* content_b = cbor_tag_item(b);
      bool equal = value_equal(content_a, content_b);
      cbor_decref(&content_a);
      cbor_decref(&content_b);
      return equal;
    }
    case CBOR_TYPE_FLOAT_CTRL: {
      if (cbor_is_float(a) != cbor_is_float(b)) return false;
      if (!cbor_is_float(a)) return cbor_ctrl_value(a) == cbor_ctrl_value(b);
      double x = cbor_float_get_float(a), y = cbor_float_get_float(b);
      if (isnan(x) && isnan(y)) return true;
      return x == y && signbit(x) == signbit(y);
    }
  }
  return false;  // LCOV_EXCL_LINE
}

/* Vector execution */

static const struct expected_failure* find_expected_failure(
    const char* file, const char* description, const char* encoded_hex) {
  size_t count = sizeof(expected_failures) / sizeof(expected_failures[0]);
  for (size_t i = 0; i < count; i++) {
    if (strcmp(expected_failures[i].file, file) == 0 &&
        strcmp(expected_failures[i].description, description) == 0 &&
        strcmp(expected_failures[i].encoded, encoded_hex) == 0) {
      return &expected_failures[i];
    }
  }
  return NULL;
}

/* Returns true if the streaming decoder consumes the whole input without
 * error. This is a weaker check than cbor_load since the streaming decoder
 * is stateless and does not track nesting. */
static bool stream_decodes(cbor_data data, size_t length) {
  size_t read = 0;
  while (read < length) {
    struct cbor_decoder_result result = cbor_stream_decode(
        data + read, length - read, &cbor_empty_callbacks, NULL);
    if (result.status != CBOR_DECODER_FINISHED) return false;
    read += result.read;
  }
  return true;
}

/* Runs one vector. Returns NULL on success, otherwise a description of the
 * failure (static storage). */
static const char* run_vector(cbor_item_t* vector, bool file_fail) {
  cbor_item_t* encoded = map_get(vector, "encoded");
  cbor_item_t* decoded = map_get(vector, "decoded");
  bool expect_fail = get_bool(vector, "fail", file_fail);
  bool roundtrip = get_bool(vector, "roundtrip", true);
  if (encoded == NULL) return "vector has no 'encoded' field";
  assert_true(cbor_isa_bytestring(encoded));
  assert_true(cbor_bytestring_is_definite(encoded));
  cbor_data data = cbor_bytestring_handle(encoded);
  size_t length = cbor_bytestring_length(encoded);

  struct cbor_load_result result;
  cbor_item_t* item = cbor_load(data, length, &result);
  bool accepted = item != NULL && result.read == length;

  if (expect_fail) {
    if (item != NULL) cbor_decref(&item);
    return accepted ? "expected rejection, but decoded successfully" : NULL;
  }

  static char failure[128];
  if (!accepted) {
    snprintf(failure, sizeof(failure), "decoding failed (error %d, read %zu)",
             item == NULL ? (int)result.error.code : 0, result.read);
    if (item != NULL) cbor_decref(&item);
    return failure;
  }

  const char* outcome = NULL;
  if (!stream_decodes(data, length)) {
    outcome = "cbor_stream_decode rejected the input";
  } else if (decoded != NULL && !value_equal(item, decoded)) {
    outcome = "decoded value differs from 'decoded'";
  } else if (roundtrip) {
    unsigned char* buffer = NULL;
    size_t buffer_size = 0;
    size_t written = cbor_serialize_alloc(item, &buffer, &buffer_size);
    if (written != length || memcmp(buffer, data, length) != 0) {
      char* hex = to_hex(buffer, written);
      snprintf(failure, sizeof(failure), "round trip produced %s", hex);
      free(hex);
      outcome = failure;
    }
    _cbor_free(buffer);
  }
  cbor_decref(&item);
  return outcome;
}

static void run_suite(const char* file) {
  size_t length;
  unsigned char* data = read_file(file, &length);
  struct cbor_load_result result;
  cbor_item_t* suite = cbor_load(data, length, &result);
  if (suite == NULL || result.read != length) {
    print_error("Cannot decode suite %s (error %d)\n", file,
                suite == NULL ? (int)result.error.code : 0);
    fail();
  }

  bool file_fail = get_bool(suite, "fail", false);
  cbor_item_t* vectors = map_get(suite, "tests");
  assert_non_null(vectors);
  assert_true(cbor_isa_array(vectors));

  for (size_t i = 0; i < cbor_array_size(vectors); i++) {
    cbor_item_t* vector = cbor_array_handle(vectors)[i];
    cbor_item_t* description_item = map_get(vector, "description");
    assert_non_null(description_item);
    size_t description_length = cbor_string_length(description_item);
    char* description = malloc(description_length + 1);
    assert_non_null(description);
    memcpy(description, cbor_string_handle(description_item),
           description_length);
    description[description_length] = '\0';

    cbor_item_t* encoded = map_get(vector, "encoded");
    char* encoded_hex = encoded == NULL
                            ? to_hex((cbor_data) "", 0)
                            : to_hex(cbor_bytestring_handle(encoded),
                                     cbor_bytestring_length(encoded));

    vectors_run++;
    const char* failure = run_vector(vector, file_fail);
    const struct expected_failure* expected =
        find_expected_failure(file, description, encoded_hex);

    if (failure != NULL && expected == NULL) {
      unexpected_failures++;
      print_message("FAIL %s: \"%s\" [%s]: %s\n", file, description,
                    encoded_hex, failure);
    } else if (failure == NULL && expected != NULL) {
      unexpected_passes++;
      print_message(
          "UNEXPECTED PASS %s: \"%s\" [%s]: remove it from "
          "expected_failures (listed reason: %s)\n",
          file, description, encoded_hex, expected->reason);
    } else if (failure != NULL) {
      expected_failures_seen++;
    }

    free(description);
    free(encoded_hex);
  }

  cbor_decref(&suite);
  free(data);
}

static void run_suites(const char* const* files, size_t count) {
  reset_counters();
  for (size_t i = 0; i < count; i++) run_suite(files[i]);
  print_message("%d vectors, %d expected failures\n", vectors_run,
                expected_failures_seen);
  assert_true(vectors_run > 0);
  if (unexpected_failures > 0 || unexpected_passes > 0) {
    print_error("%d unexpected failures, %d unexpected passes\n",
                unexpected_failures, unexpected_passes);
    fail();
  }
}

static void test_rfc8949_appendix_a(void** _state _CBOR_UNUSED) {
  static const char* const files[] = {
      "rfc8949-appendixA/mt0.cbor",        "rfc8949-appendixA/mt1.cbor",
      "rfc8949-appendixA/mt2.cbor",        "rfc8949-appendixA/mt3.cbor",
      "rfc8949-appendixA/mt4.cbor",        "rfc8949-appendixA/mt5.cbor",
      "rfc8949-appendixA/mt6.cbor",        "rfc8949-appendixA/mt7-float.cbor",
      "rfc8949-appendixA/mt7-simple.cbor", "rfc8949-appendixA/streaming.cbor",
  };
  run_suites(files, sizeof(files) / sizeof(files[0]));
}

static void test_rfc8949_good(void** _state _CBOR_UNUSED) {
  static const char* const files[] = {"rfc8949/good.cbor"};
  run_suites(files, 1);
}

static void test_rfc8949_bad(void** _state _CBOR_UNUSED) {
  static const char* const files[] = {"rfc8949/bad.cbor"};
  run_suites(files, 1);
}

static void test_spike(void** _state _CBOR_UNUSED) {
  static const char* const files[] = {"spike/spike.cbor"};
  run_suites(files, 1);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_rfc8949_appendix_a),
      cmocka_unit_test(test_rfc8949_good),
      cmocka_unit_test(test_rfc8949_bad),
      cmocka_unit_test(test_spike),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
