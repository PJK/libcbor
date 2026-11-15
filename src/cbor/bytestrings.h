/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBCBOR_BYTESTRINGS_H
#define LIBCBOR_BYTESTRINGS_H

#include "cbor/cbor_export.h"
#include "cbor/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ============================================================================
 * Byte string manipulation
 * ============================================================================
 */

/** Returns the length of the binary data
 *
 * For definite byte strings only
 *
 * @param item a definite bytestring
 * @return length of the binary data. Zero if no chunk has been attached yet
 */
_CBOR_NODISCARD
CBOR_EXPORT size_t cbor_bytestring_length(const cbor_item_t* item);

/** Is the byte string definite?
 *
 * @param item a byte string
 * @return Is the byte string definite?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_bytestring_is_definite(const cbor_item_t* item);

/** Is the byte string indefinite?
 *
 * @param item a byte string
 * @return Is the byte string indefinite?
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_bytestring_is_indefinite(const cbor_item_t* item);

/** Get the handle to the binary data
 *
 * Definite items only. Modifying the data is allowed. In that case, the caller
 * takes responsibility for the effect on items this item might be a part of
 *
 * @param item A definite byte string
 * @return The address of the underlying binary data
 * @return `NULL` if no data have been assigned yet or if the byte string is
 * empty (no storage is allocated for zero-length data)
 */
_CBOR_NODISCARD
CBOR_EXPORT cbor_mutable_data cbor_bytestring_handle(const cbor_item_t* item);

/** Set the handle to the binary data
 *
 * \rst
 * .. warning::
 *   Any buffer previously attached to ``item`` is *not* freed. Calling this on
 *   an item that already owns a buffer (e.g., created with
 *   :func:`cbor_build_bytestring`) will leak the previous buffer unless the
 *   caller frees :func:`cbor_bytestring_handle` first.
 * \endrst
 *
 * @param item A definite byte string
 * @param data The memory block, allocated with the `malloc` implementation
 * configured using `cbor_set_allocs` (`malloc` by default). The caller gives
 * up the ownership of the block; libcbor will deallocate it when appropriate
 * using the matching `free` implementation.
 * @param length Length of the data block. If 0, `data` may be `NULL`
 */
CBOR_EXPORT void cbor_bytestring_set_handle(
    cbor_item_t* item, cbor_mutable_data CBOR_RESTRICT_POINTER data,
    size_t length);

/** Get the handle to the array of chunks
 *
 * Returns a borrowed pointer to the internal chunk array without modifying any
 * reference counts. Manipulations with the memory block (e.g. sorting it) are
 * allowed as long as reference counts and #cbor_bytestring_chunk_count remain
 * consistent. The returned pointer is invalidated if
 * #cbor_bytestring_add_chunk reallocates the array or \p item is deallocated.
 *
 * @param item An indefinite byte string
 * @return Array of #cbor_bytestring_chunk_count definite bytestrings, or `NULL`
 * if no chunks have been added yet
 */
_CBOR_NODISCARD
CBOR_EXPORT cbor_item_t** cbor_bytestring_chunks_handle(
    const cbor_item_t* item);

/** Get the number of chunks this string consist of
 *
 * @param item A indefinite bytestring
 * @return The chunk count. 0 for freshly created items.
 */
_CBOR_NODISCARD
CBOR_EXPORT size_t cbor_bytestring_chunk_count(const cbor_item_t* item);

/** Appends a chunk to the bytestring
 *
 * Indefinite byte strings only.
 *
 * May realloc the chunk storage.
 *
 * @param item An indefinite byte string
 * @param chunk A definite byte string. Its reference count will be increased
 * by one on success.
 * @return `true` on success, `false` on realloc failure. In that case, the
 * refcount of \p chunk is not increased and \p item is left intact.
 */
_CBOR_NODISCARD
CBOR_EXPORT bool cbor_bytestring_add_chunk(cbor_item_t* item,
                                           cbor_item_t* chunk);

/** Creates a new definite byte string
 *
 * The handle is initialized to `NULL` and length to 0
 *
 * @return Reference to the new bytestring item. The item's reference count is
 * initialized to one and must be released using `cbor_decref`.
 * @return `NULL` if memory allocation fails
 */
_CBOR_NODISCARD
CBOR_EXPORT cbor_item_t* cbor_new_definite_bytestring(void);

/** Creates a new indefinite byte string
 *
 * The chunks array is initialized to `NULL` and chunk count to 0
 *
 * @return Reference to the new bytestring item. The item's reference count is
 * initialized to one and must be released using `cbor_decref`.
 * @return `NULL` if memory allocation fails
 */
_CBOR_NODISCARD
CBOR_EXPORT cbor_item_t* cbor_new_indefinite_bytestring(void);

/** Creates a new byte string and initializes it
 *
 * The \p handle data will be copied to a newly allocated block
 *
 * @param handle Block of binary data
 * @param length Length of \p handle
 * @return Reference to the new bytestring item. The item's reference count is
 * initialized to one and must be released using `cbor_decref`.
 * @return `NULL` if memory allocation fails
 */
_CBOR_NODISCARD
CBOR_EXPORT cbor_item_t* cbor_build_bytestring(cbor_data handle, size_t length);

#ifdef __cplusplus
}
#endif

#endif  // LIBCBOR_BYTESTRINGS_H
