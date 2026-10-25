/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBCBOR_ENCODERS_H
#define LIBCBOR_ENCODERS_H

#include "cbor/common.h"

#ifdef __cplusplus
extern "C" {
#endif

_CBOR_NODISCARD
size_t _cbor_encode_uint8(uint8_t value, unsigned char* buffer,
                          size_t buffer_size, uint8_t offset);

_CBOR_NODISCARD
size_t _cbor_encode_uint16(uint16_t value, unsigned char* buffer,
                           size_t buffer_size, uint8_t offset);

_CBOR_NODISCARD
size_t _cbor_encode_uint32(uint32_t value, unsigned char* buffer,
                           size_t buffer_size, uint8_t offset);

_CBOR_NODISCARD
size_t _cbor_encode_uint64(uint64_t value, unsigned char* buffer,
                           size_t buffer_size, uint8_t offset);

_CBOR_NODISCARD
size_t _cbor_encode_uint(uint64_t value, unsigned char* buffer,
                         size_t buffer_size, uint8_t offset);

#ifdef __cplusplus
}
#endif

#endif  // LIBCBOR_ENCODERS_H
