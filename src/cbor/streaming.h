/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBCBOR_STREAMING_H
#define LIBCBOR_STREAMING_H

#include "callbacks.h"
#include "cbor/cbor_export.h"
#include "cbor/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Stateless decoder
 *
 * Will try parsing the \p source and will invoke the appropriate callback on
 * success. Decodes one item at a time. No memory allocations occur.
 *
 * @param source Input buffer
 * @param source_size Length of the buffer
 * @param callbacks The callback bundle
 * @param context An arbitrary pointer to allow for maintaining context.
 */
_CBOR_NODISCARD CBOR_EXPORT struct cbor_decoder_result cbor_stream_decode(
    cbor_data source, size_t source_size,
    const struct cbor_callbacks* callbacks, void* context);

#ifdef __cplusplus
}
#endif

#endif  // LIBCBOR_STREAMING_H
