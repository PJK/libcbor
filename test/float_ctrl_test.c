/*
 * SPDX-FileCopyrightText: 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <tgmath.h>

#include <cmocka.h>

#include "cbor.h"
#include "test_allocator.h"

cbor_item_t* float_ctrl;
struct cbor_load_result res;

static const float eps = 0.00001f;

unsigned char float2_data[] = {0xF9, 0x7B, 0xFF};

static void test_float2(void** _state _CBOR_UNUSED) {
  float_ctrl = cbor_load(float2_data, 3, &res);
  assert_true(cbor_isa_float_ctrl(float_ctrl));
  assert_true(cbor_is_float(float_ctrl));
  assert_true(cbor_float_get_width(float_ctrl) == CBOR_FLOAT_16);
  assert_true(cbor_float_get_float2(float_ctrl) == 65504.0F);
  assert_float_equal(cbor_float_get_float(float_ctrl), 65504.0F, eps);
  cbor_decref(&float_ctrl);
  assert_null(float_ctrl);
}

/* Bit pattern of a float, to compare results exactly: distinguishes -0.0 from
 * 0.0 and exposes NaN payloads. */
static uint32_t float_bits(float value) {
  uint32_t bits;
  memcpy(&bits, &value, sizeof(bits));
  return bits;
}

/* Decodes the half-precision float with the given bit pattern */
static float load_half(uint16_t half) {
  unsigned char data[] = {0xF9, (unsigned char)(half >> 8),
                          (unsigned char)(half & 0xFF)};
  float_ctrl = cbor_load(data, 3, &res);
  assert_non_null(float_ctrl);
  assert_size_equal(res.read, 3);
  assert_true(cbor_float_get_width(float_ctrl) == CBOR_FLOAT_16);
  float value = cbor_float_get_float2(float_ctrl);
  cbor_decref(&float_ctrl);
  return value;
}

/* The boundaries of each class of half-precision numbers */
static const struct {
  uint16_t half;
  float value;
} float2_values[] = {
    {0x0000, 0.0f},
    {0x8000, -0.0f},
    {0x0001, 5.9604644775390625e-8f}, /* Smallest subnormal, 2^-24 */
    {0x8001, -5.9604644775390625e-8f},
    {0x0002, 1.1920928955078125e-7f},
    {0x03FF, 6.0975551605224609375e-5f}, /* Largest subnormal */
    {0x0400, 6.103515625e-5f},           /* Smallest normal, 2^-14 */
    {0x0401, 6.1094760894775390625e-5f},
    {0x3BFF, 0.99951171875f}, /* Largest below one */
    {0x3C00, 1.0f},
    {0x3C01, 1.0009765625f}, /* Smallest above one */
    {0x3E00, 1.5f},
    {0x4000, 2.0f},
    {0xC400, -4.0f},
    {0x5640, 100.0f},
    {0x7BFF, 65504.0f}, /* Largest normal */
    {0xFBFF, -65504.0f},
    {0x7C00, INFINITY},
    {0xFC00, -INFINITY},
};

static void test_float2_values(void** _state _CBOR_UNUSED) {
  for (size_t i = 0; i < sizeof(float2_values) / sizeof(float2_values[0]);
       i++) {
    assert_int_equal(float_bits(load_half(float2_values[i].half)),
                     float_bits(float2_values[i].value));
  }
}

static void test_float2_nan(void** _state _CBOR_UNUSED) {
  assert_int_equal(float_bits(load_half(0x7E00)), 0x7FC00000);
  assert_int_equal(float_bits(load_half(0xFE00)), 0xFFC00000);
  assert_int_equal(float_bits(load_half(0x7FFF)), 0x7FFFE000);
  assert_int_equal(float_bits(load_half(0xFE01)), 0xFFC02000);
}

/* Value of a half-precision number per IEEE 754: (-1)^sign * 2^(exponent -
 * 15) * 1.fraction for normal numbers, (-1)^sign * 2^-14 * 0.fraction for
 * subnormal numbers. Only valid for finite numbers. */
static double float2_reference(uint16_t half) {
  int exponent = (half >> 10) & 0x1F;
  double fraction = (half & 0x3FF) / 1024.0;
  double magnitude = exponent == 0 ? ldexp(fraction, -14)
                                   : ldexp(1.0 + fraction, exponent - 15);
  return (half & 0x8000) ? -magnitude : magnitude;
}

/* Every half-precision value is exactly representable as a float, so decoding
 * must be exact for all 2^16 bit patterns, and encoding the result must give
 * back the original bytes. */
static void test_float2_all_values(void** _state _CBOR_UNUSED) {
  for (uint32_t i = 0; i <= 0xFFFF; i++) {
    uint16_t half = (uint16_t)i;
    uint32_t sign = (uint32_t)(half & 0x8000) << 16;
    uint32_t exponent = (half >> 10) & 0x1F;
    uint32_t fraction = half & 0x3FF;
    float value = load_half(half);
    uint32_t bits = float_bits(value);

    uint32_t expected;
    // Quiet bit that may additionally be set on signaling NaNs
    uint32_t tolerated = 0;
    if (exponent == 0x1F && fraction == 0) {
      expected = sign | 0x7F800000u;
    } else if (exponent == 0x1F) {
      // NaN: the sign and the payload keep their positions
      expected = sign | 0x7F800000u | fraction << 13;
#if defined(__i386__) || defined(_M_IX86)
      // x87 quiets signaling NaNs when loading them into a register. See
      // https://github.com/PJK/libcbor/issues/215
      if (!(fraction & 0x200)) tolerated = 0x00400000u;
#endif
    } else {
      expected = float_bits((float)float2_reference(half));
    }
    if (bits != expected && bits != (expected | tolerated)) {
      print_error("0x%04X decoded to 0x%08lX, expected 0x%08lX\n",
                  (unsigned)half, (unsigned long)bits, (unsigned long)expected);
      fail();
    }

    unsigned char encoded[3];
    assert_size_equal(cbor_encode_half(value, encoded, 3), 3);
    uint16_t roundtrip = (uint16_t)(encoded[1] << 8 | encoded[2]);
    if (roundtrip != half && roundtrip != (half | (tolerated >> 13))) {
      print_error("0x%04X encoded back as 0x%04X\n", (unsigned)half,
                  (unsigned)roundtrip);
      fail();
    }
  }
}

unsigned char float4_data[] = {0xFA, 0x47, 0xC3, 0x50, 0x00};

static void test_float4(void** _state _CBOR_UNUSED) {
  float_ctrl = cbor_load(float4_data, 5, &res);
  assert_true(cbor_isa_float_ctrl(float_ctrl));
  assert_true(cbor_is_float(float_ctrl));
  assert_true(cbor_float_get_width(float_ctrl) == CBOR_FLOAT_32);
  assert_true(cbor_float_get_float4(float_ctrl) == 100000.0F);
  assert_float_equal(cbor_float_get_float(float_ctrl), 100000.0F, eps);
  cbor_decref(&float_ctrl);
  assert_null(float_ctrl);
}

unsigned char float8_data[] = {0xFB, 0x7E, 0x37, 0xE4, 0x3C,
                               0x88, 0x00, 0x75, 0x9C};

static void test_float8(void** _state _CBOR_UNUSED) {
  float_ctrl = cbor_load(float8_data, 9, &res);
  assert_true(cbor_isa_float_ctrl(float_ctrl));
  assert_true(cbor_is_float(float_ctrl));
  assert_true(cbor_float_get_width(float_ctrl) == CBOR_FLOAT_64);
  // XXX: the cast prevents promotion to 80-bit floats on 32-bit x86
  assert_true(cbor_float_get_float8(float_ctrl) == (double)1.0e+300);
  // Not using `assert_double_equal` since CI has an old version of cmocka
  assert_true(fabs(cbor_float_get_float(float_ctrl) - (double)1.0e+300) < eps);
  cbor_decref(&float_ctrl);
  assert_null(float_ctrl);
}

unsigned char null_data[] = {0xF6};

static void test_null(void** _state _CBOR_UNUSED) {
  float_ctrl = cbor_load(null_data, 1, &res);
  assert_true(cbor_isa_float_ctrl(float_ctrl));
  assert_true(cbor_float_ctrl_is_ctrl(float_ctrl));
  assert_true(cbor_float_get_width(float_ctrl) == CBOR_FLOAT_0);
  assert_true(cbor_is_null(float_ctrl));
  cbor_decref(&float_ctrl);
  assert_null(float_ctrl);
}

unsigned char undef_data[] = {0xF7};

static void test_undef(void** _state _CBOR_UNUSED) {
  float_ctrl = cbor_load(undef_data, 1, &res);
  assert_true(cbor_isa_float_ctrl(float_ctrl));
  assert_true(cbor_float_ctrl_is_ctrl(float_ctrl));
  assert_true(cbor_float_get_width(float_ctrl) == CBOR_FLOAT_0);
  assert_true(cbor_is_undef(float_ctrl));
  cbor_decref(&float_ctrl);
  assert_null(float_ctrl);
}

unsigned char simple_value_data[][2] = {
    {0xE0, 0x00}, {0xF3, 0x00}, {0xF8, 0x20}, {0xF8, 0xFF}};
size_t simple_value_lengths[] = {1, 1, 2, 2};
uint8_t simple_values[] = {0, 19, 32, 255};

/* Unassigned simple values are well-formed and must round-trip */
static void test_simple_values(void** _state _CBOR_UNUSED) {
  for (size_t i = 0; i < 4; i++) {
    float_ctrl = cbor_load(simple_value_data[i], simple_value_lengths[i], &res);
    assert_non_null(float_ctrl);
    assert_size_equal(res.read, simple_value_lengths[i]);
    assert_true(cbor_isa_float_ctrl(float_ctrl));
    assert_true(cbor_float_ctrl_is_ctrl(float_ctrl));
    assert_true(cbor_float_get_width(float_ctrl) == CBOR_FLOAT_0);
    assert_false(cbor_is_bool(float_ctrl));
    assert_false(cbor_is_null(float_ctrl));
    assert_false(cbor_is_undef(float_ctrl));
    assert_int_equal(cbor_ctrl_value(float_ctrl), simple_values[i]);

    unsigned char buffer[2];
    assert_size_equal(cbor_serialize(float_ctrl, buffer, 2),
                      simple_value_lengths[i]);
    assert_memory_equal(buffer, simple_value_data[i], simple_value_lengths[i]);

    cbor_decref(&float_ctrl);
    assert_null(float_ctrl);
  }
}

static void test_simple_value_alloc_failure(void** _state _CBOR_UNUSED) {
  WITH_FAILING_MALLOC({
    assert_null(cbor_load(simple_value_data[0], 1, &res));
    assert_true(res.error.code == CBOR_ERR_MEMERROR);
  });
}

/* 0xF8 followed by a byte below 0x20 is not well-formed */
unsigned char reserved_simple_value_data[] = {0xF8, 0x1F};
static void test_reserved_simple_value(void** _state _CBOR_UNUSED) {
  assert_null(cbor_load(reserved_simple_value_data, 2, &res));
  assert_true(res.error.code == CBOR_ERR_MALFORMATED);
}

unsigned char bool_data[] = {0xF4, 0xF5};

static void test_bool(void** _state _CBOR_UNUSED) {
  _CBOR_TEST_DISABLE_ASSERT({
    float_ctrl = cbor_load(bool_data, 1, &res);
    assert_true(cbor_isa_float_ctrl(float_ctrl));
    assert_true(cbor_float_ctrl_is_ctrl(float_ctrl));
    assert_true(cbor_float_get_width(float_ctrl) == CBOR_FLOAT_0);
    assert_true(cbor_is_bool(float_ctrl));
    assert_false(cbor_get_bool(float_ctrl));
    cbor_set_bool(float_ctrl, true);
    assert_true(cbor_get_bool(float_ctrl));
    assert_true(isnan(cbor_float_get_float(float_ctrl)));
    cbor_decref(&float_ctrl);
    assert_null(float_ctrl);

    float_ctrl = cbor_load(bool_data + 1, 1, &res);
    assert_true(cbor_isa_float_ctrl(float_ctrl));
    assert_true(cbor_float_ctrl_is_ctrl(float_ctrl));
    assert_true(cbor_float_get_width(float_ctrl) == CBOR_FLOAT_0);
    assert_true(cbor_is_bool(float_ctrl));
    assert_true(cbor_get_bool(float_ctrl));
    cbor_set_bool(float_ctrl, false);
    assert_false(cbor_get_bool(float_ctrl));
    assert_true(isnan(cbor_float_get_float(float_ctrl)));
    cbor_decref(&float_ctrl);
    assert_null(float_ctrl);
  });
}

static void test_float_ctrl_creation(void** _state _CBOR_UNUSED) {
  WITH_FAILING_MALLOC({ assert_null(cbor_new_ctrl()); });
  WITH_FAILING_MALLOC({ assert_null(cbor_new_float2()); });
  WITH_FAILING_MALLOC({ assert_null(cbor_new_float4()); });
  WITH_FAILING_MALLOC({ assert_null(cbor_new_float8()); });
  WITH_FAILING_MALLOC({ assert_null(cbor_new_null()); });
  WITH_FAILING_MALLOC({ assert_null(cbor_new_undef()); });

  WITH_FAILING_MALLOC({ assert_null(cbor_build_bool(false)); });
  WITH_FAILING_MALLOC({ assert_null(cbor_build_float2(3.14f)); });
  WITH_FAILING_MALLOC({ assert_null(cbor_build_float4(3.14f)); });
  WITH_FAILING_MALLOC({ assert_null(cbor_build_float8(3.14)); });
  WITH_FAILING_MALLOC({ assert_null(cbor_build_ctrl(0xAF)); });
}

static void test_ctrl_on_float(void** _state _CBOR_UNUSED) {
  float_ctrl = cbor_build_float4(3.14f);
  assert_non_null(float_ctrl);
  assert_true(cbor_is_float(float_ctrl));
  assert_false(cbor_float_ctrl_is_ctrl(float_ctrl));
  assert_false(cbor_is_null(float_ctrl));
  assert_false(cbor_is_undef(float_ctrl));
  assert_false(cbor_is_bool(float_ctrl));
  cbor_decref(&float_ctrl);
  assert_null(float_ctrl);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_float2),
      cmocka_unit_test(test_float2_values),
      cmocka_unit_test(test_float2_nan),
      cmocka_unit_test(test_float2_all_values),
      cmocka_unit_test(test_float4),
      cmocka_unit_test(test_float8),
      cmocka_unit_test(test_null),
      cmocka_unit_test(test_undef),
      cmocka_unit_test(test_simple_values),
      cmocka_unit_test(test_simple_value_alloc_failure),
      cmocka_unit_test(test_reserved_simple_value),
      cmocka_unit_test(test_bool),
      cmocka_unit_test(test_float_ctrl_creation),
      cmocka_unit_test(test_ctrl_on_float),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
