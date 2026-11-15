/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBCBOR_ARRAY_UTILS_H
#define LIBCBOR_ARRAY_UTILS_H

#include "cbor/cbor_export.h"
#include "cbor/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Grow the storage of a full definite array
 *
 * For the decoder, which preallocates less than the declared size when the
 * input cannot hold it. Grows by `CBOR_BUFFER_GROWTH`, to at most \p limit
 * items. New slots are `NULL`.
 *
 * @param array A definite array with no free storage
 * @param limit Maximum number of items, greater than the current size
 * @return `false` if the reallocation failed
 */
_CBOR_NODISCARD CBOR_NO_EXPORT _CBOR_COLD bool _cbor_array_grow_definite(
    cbor_item_t* array, size_t limit);

#ifdef __cplusplus
}
#endif

#endif  // LIBCBOR_ARRAY_UTILS_H
