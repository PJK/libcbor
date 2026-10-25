/*
 * SPDX-FileCopyrightText: 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#include "loaders.h"
#include <string.h>

uint8_t _cbor_load_uint8(cbor_data source) { return (uint8_t)*source; }

uint16_t _cbor_load_uint16(const unsigned char* source) {
#ifdef IS_BIG_ENDIAN
  uint16_t result;
  memcpy(&result, source, 2);
  return result;
#else
  return ((uint16_t) * (source + 0) << 8) + (uint8_t) * (source + 1);
#endif
}

uint32_t _cbor_load_uint32(const unsigned char* source) {
#ifdef IS_BIG_ENDIAN
  uint32_t result;
  memcpy(&result, source, 4);
  return result;
#else
  return ((uint32_t) * (source + 0) << 0x18) +
         ((uint32_t) * (source + 1) << 0x10) +
         ((uint16_t) * (source + 2) << 0x08) + (uint8_t) * (source + 3);
#endif
}

uint64_t _cbor_load_uint64(const unsigned char* source) {
#ifdef IS_BIG_ENDIAN
  uint64_t result;
  memcpy(&result, source, 8);
  return result;
#else
  return ((uint64_t) * (source + 0) << 0x38) +
         ((uint64_t) * (source + 1) << 0x30) +
         ((uint64_t) * (source + 2) << 0x28) +
         ((uint64_t) * (source + 3) << 0x20) +
         ((uint32_t) * (source + 4) << 0x18) +
         ((uint32_t) * (source + 5) << 0x10) +
         ((uint16_t) * (source + 6) << 0x08) + (uint8_t) * (source + 7);
#endif
}

/* Converts an IEEE 754 binary16 value to binary32, which represents every
 * binary16 value exactly, by moving its fields into place: the sign bit, the
 * exponent rebiased from 15 to 127, and the 10-bit fraction as the top of the
 * 23-bit fraction. Binary32 represents binary16 subnormals as normal numbers,
 * so those are normalized first. Infinities and NaNs keep their sign and
 * payload, which matches cbor_encode_half and makes NaNs round-trip. */
float _cbor_decode_half(unsigned char* halfp) {
  // TODO: Broken if we are not on IEEE 754
  // (https://github.com/PJK/libcbor/issues/336)
  uint16_t half = (uint16_t)(halfp[0] << 8 | halfp[1]);
  uint32_t sign = (uint32_t)(half & 0x8000u) << 16;
  uint32_t exponent = (half >> 10) & 0x1Fu;
  uint32_t fraction = half & 0x3FFu;

  if (exponent == 0x1F) {
    /* Infinity or NaN */
    exponent = 0xFF;
  } else if (exponent != 0) {
    exponent += 127 - 15;
  } else if (fraction != 0) {
    /* Subnormal: shift the leading one into the implicit bit, adjusting the
     * exponent of the smallest normal number (2^-14) to match */
    exponent = 127 - 14;
    while (!(fraction & 0x400u)) {
      fraction <<= 1;
      exponent--;
    }
    fraction &= 0x3FFu;
  } /* Otherwise zero: the exponent and the fraction are zero, too */

  union _cbor_float_helper helper = {.as_uint = sign | (exponent << 23) |
                                                (fraction << 13)};
  return helper.as_float;
}

float _cbor_load_half(cbor_data source) {
  /* Discard const */
  return _cbor_decode_half((unsigned char*)source);
}

float _cbor_load_float(cbor_data source) {
  // TODO: Broken if we are not on IEEE 754
  // (https://github.com/PJK/libcbor/issues/336)
  union _cbor_float_helper helper = {.as_uint = _cbor_load_uint32(source)};
  return helper.as_float;
}

double _cbor_load_double(cbor_data source) {
  // TODO: Broken if we are not on IEEE 754
  // (https://github.com/PJK/libcbor/issues/336)
  union _cbor_double_helper helper = {.as_uint = _cbor_load_uint64(source)};
  return helper.as_double;
}
