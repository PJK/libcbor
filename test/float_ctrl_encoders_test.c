/*
 * Copyright (c) 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <math.h>
#include <string.h>
#include "assertions.h"
#include "cbor.h"

unsigned char buffer[512];

static void test_bools(void** _state _CBOR_UNUSED) {
  assert_size_equal(1, cbor_encode_bool(false, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF4}), 1);
  assert_size_equal(1, cbor_encode_bool(true, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF5}), 1);
}

static void test_null(void** _state _CBOR_UNUSED) {
  assert_size_equal(1, cbor_encode_null(buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF6}), 1);
}

static void test_undef(void** _state _CBOR_UNUSED) {
  assert_size_equal(1, cbor_encode_undef(buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF7}), 1);
}

static void test_break(void** _state _CBOR_UNUSED) {
  assert_size_equal(1, cbor_encode_break(buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xFF}), 1);
}

/* Check that encode(decode(buffer)) = buffer for a valid half-float in the
 * buffer.*/
static void assert_half_float_codec_identity(void) {
  unsigned char secondary_buffer[3];
  struct cbor_load_result res;
  // Load and check data in buffer
  cbor_item_t* half_float = cbor_load(buffer, 3, &res);
  assert_size_equal(res.error.code, CBOR_ERR_NONE);
  assert_true(cbor_isa_float_ctrl(half_float));
  assert_true(cbor_is_float(half_float));
  assert_size_equal(cbor_float_get_width(half_float), CBOR_FLOAT_16);
  // Encode again and check equality
  assert_size_equal(3, cbor_encode_half(cbor_float_get_float2(half_float),
                                        secondary_buffer, 3));
  assert_memory_equal(buffer, secondary_buffer, 3);
  cbor_decref(&half_float);
}

static void test_half(void** _state _CBOR_UNUSED) {
  assert_size_equal(3, cbor_encode_half(1.5f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x3E, 0x00}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(-0.0f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x80, 0x00}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(0.0f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x00, 0x00}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(65504.0f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x7B, 0xFF}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(0.00006103515625f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x04, 0x00}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(-4.0f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0xC4, 0x00}), 3);
  assert_half_float_codec_identity();

  /* Smallest representable value */
  assert_size_equal(3, cbor_encode_half(5.960464477539063e-8f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x00, 0x01}), 3);
  assert_half_float_codec_identity();

  /* Smaller than the smallest, approximate magnitude representation */
  assert_size_equal(3, cbor_encode_half(5.960464477539062e-8f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x00, 0x01}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(4.172325134277344e-7f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x00, 0x07}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(6.097555160522461e-5f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x03, 0xff}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(6.100535392761231e-5f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x04, 0x00}), 3);
  assert_half_float_codec_identity();

  /* Smaller than the smallest and even the magnitude cannot be represented,
     round off to zero. Sign bit is preserved. */
  assert_size_equal(3, cbor_encode_half(1e-25f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x00, 0x00}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(-1e-25f, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x80, 0x00}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(1.1920928955078125e-7, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x00, 0x02}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(-1.1920928955078124e-7, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x80, 0x02}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(INFINITY, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x7C, 0x00}), 3);
  assert_half_float_codec_identity();
}

/* Check that buffer holds a valid half-precision CBOR NaN with the expected
 * sign (0 = positive, non-zero = negative), then verify encode/decode
 * round-trip. The exact payload bits are not checked because the C NAN
 * constant has platform-specific bit patterns (e.g. MIPS legacy NaN). */
static void assert_half_nan_and_roundtrip(int negative) {
  /* CBOR additional info 0xF9 = half-precision float */
  assert_int_equal(buffer[0], 0xF9);
  /* Sign bit */
  if (negative) {
    assert_true(buffer[1] & 0x80);
  } else {
    assert_false(buffer[1] & 0x80);
  }
  /* All five exponent bits must be set (= 0x1F) for NaN/Inf */
  assert_true((buffer[1] & 0x7C) == 0x7C);
  /* Mantissa must be non-zero to distinguish NaN from Infinity */
  assert_true((buffer[1] & 0x03) || buffer[2]);
  assert_half_float_codec_identity();
}

static void test_half_special(void** _state _CBOR_UNUSED) {
  /* The C NAN constant has platform-specific bits (e.g. MIPS legacy NaN
   * uses bit 22 = 0 for quiet NaN, opposite of IEEE 754-2008). We only
   * check that the result is a structurally valid half-precision NaN and
   * that it round-trips correctly. */
  assert_size_equal(3, cbor_encode_half(NAN, buffer, 512));
  assert_half_nan_and_roundtrip(0);

  assert_size_equal(3, cbor_encode_half(nanf("2"), buffer, 512));
  assert_half_nan_and_roundtrip(0);

  /* Negative NaN: sign bit must be preserved. */
  assert_size_equal(3, cbor_encode_half(-NAN, buffer, 512));
  assert_half_nan_and_roundtrip(1);

  /* NaN with payload bits in the top 10 of the mantissa: payload is
   * preserved in half precision. Bit pattern 0x7FC02000 has the IEEE quiet
   * bit (bit 22) and bit 13 set; after >> 13 the half mantissa is 0x201.
   * This specific bit pattern is used (rather than a C NaN constant) to get
   * deterministic results across platforms. */
  float nan_with_payload;
  uint32_t nan_bits = 0x7FC02000u;
  memcpy(&nan_with_payload, &nan_bits, sizeof(nan_with_payload));
  assert_size_equal(3, cbor_encode_half(nan_with_payload, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x7E, 0x01}), 3);
  assert_half_float_codec_identity();
}

static void test_half_infinity(void** _state _CBOR_UNUSED) {
  assert_size_equal(3, cbor_encode_half(INFINITY, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0x7C, 0x00}), 3);
  assert_half_float_codec_identity();

  assert_size_equal(3, cbor_encode_half(-INFINITY, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xF9, 0xFC, 0x00}), 3);
  assert_half_float_codec_identity();
}

static void test_float(void** _state _CBOR_UNUSED) {
  assert_size_equal(5, cbor_encode_single(3.4028234663852886e+38, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xFA, 0x7F, 0x7F, 0xFF, 0xFF}),
                      5);

  assert_size_equal(5, cbor_encode_single(NAN, buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xFA, 0x7F, 0xC0, 0x00, 0x00}),
                      5);

  assert_size_equal(5, cbor_encode_single(nanf("3"), buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xFA, 0x7F, 0xC0, 0x00, 0x00}),
                      5);

  assert_size_equal(5, cbor_encode_single(strtof("Inf", NULL), buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xFA, 0x7F, 0x80, 0x00, 0x00}),
                      5);

  assert_size_equal(5, cbor_encode_single(strtof("-Inf", NULL), buffer, 512));
  assert_memory_equal(buffer, ((unsigned char[]){0xFA, 0xFF, 0x80, 0x00, 0x00}),
                      5);
}

static void test_double(void** _state _CBOR_UNUSED) {
  assert_size_equal(9, cbor_encode_double(1.0e+300, buffer, 512));
  assert_memory_equal(
      buffer,
      ((unsigned char[]){0xFB, 0x7E, 0x37, 0xE4, 0x3C, 0x88, 0x00, 0x75, 0x9C}),
      9);

  assert_size_equal(9, cbor_encode_double(nan(""), buffer, 512));
  assert_memory_equal(
      buffer,
      ((unsigned char[]){0xFB, 0x7F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}),
      9);

  assert_size_equal(9, cbor_encode_double(nan("3"), buffer, 512));
  assert_memory_equal(
      buffer,
      ((unsigned char[]){0xFB, 0x7F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}),
      9);

  assert_size_equal(9, cbor_encode_double(strtod("Inf", NULL), buffer, 512));
  assert_memory_equal(
      buffer,
      ((unsigned char[]){0xFB, 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}),
      9);

  assert_size_equal(9, cbor_encode_double(strtod("-Inf", NULL), buffer, 512));
  assert_memory_equal(
      buffer,
      ((unsigned char[]){0xFB, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}),
      9);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_bools),         cmocka_unit_test(test_null),
      cmocka_unit_test(test_undef),         cmocka_unit_test(test_break),
      cmocka_unit_test(test_half),          cmocka_unit_test(test_float),
      cmocka_unit_test(test_double),        cmocka_unit_test(test_half_special),
      cmocka_unit_test(test_half_infinity),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
