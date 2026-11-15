/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBCBOR_MEMORY_UTILS_H
#define LIBCBOR_MEMORY_UTILS_H

#include <stdbool.h>
#include <string.h>

#include "cbor/common.h"

/** Can `a` and `b` be multiplied without overflowing size_t? */
_CBOR_NODISCARD
bool _cbor_safe_to_multiply(size_t a, size_t b);

/** Can `a` and `b` be added without overflowing size_t? */
_CBOR_NODISCARD
bool _cbor_safe_to_add(size_t a, size_t b);

/** Adds `a` and `b`, propagating zeros and returning 0 on overflow. */
_CBOR_NODISCARD
size_t _cbor_safe_signaling_add(size_t a, size_t b);

/** Capacity to grow a full buffer to
 *
 * @param capacity Current capacity
 * @param limit Maximum capacity, greater than \p capacity
 * @return \p capacity times `CBOR_BUFFER_GROWTH`, but at least
 * \p capacity + 1 and at most \p limit
 */
_CBOR_NODISCARD
size_t _cbor_grown_capacity(size_t capacity, size_t limit);

/** Overflow-proof contiguous array allocation
 *
 * @param item_size
 * @param item_count
 * @return Region of item_size * item_count bytes, or NULL if the total size
 * overflows size_t or the underlying allocator failed
 */
void* _cbor_alloc_multiple(size_t item_size, size_t item_count);

/** Overflow-proof contiguous array reallocation
 *
 * This implements the OpenBSD `reallocarray` functionality.
 *
 * @param pointer
 * @param item_size
 * @param item_count
 * @return Realloc'd of item_size * item_count bytes, or NULL if the total size
 * overflows size_t or the underlying allocator failed
 */
void* _cbor_realloc_multiple(void* pointer, size_t item_size,
                             size_t item_count);

#endif  // LIBCBOR_MEMORY_UTILS_H
