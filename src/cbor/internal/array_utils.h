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

/** Enlarge the storage of a full definite array
 *
 * Definite arrays only accept as many items as they have storage for. The
 * decoder preallocates less than the declared size when the input cannot hold
 * that many items, and uses this to make room as the items actually arrive.
 * The storage grows by a factor of `CBOR_BUFFER_GROWTH`, but never past
 * \p limit items. The new slots are set to `NULL`. Not exported from the
 * shared library.
 *
 * @param array A definite array with no free storage
 * @param limit Maximum number of items to make room for. Must be greater than
 * the current size.
 * @return `true` on success, `false` if the reallocation failed
 */
_CBOR_NODISCARD CBOR_NO_EXPORT _CBOR_COLD bool _cbor_array_grow_definite(
    cbor_item_t* array, size_t limit);

#ifdef __cplusplus
}
#endif

#endif  // LIBCBOR_ARRAY_UTILS_H
