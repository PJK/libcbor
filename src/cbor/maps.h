/*
 * Copyright (c) 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_MAPS_H
#define LIBCBOR_MAPS_H

#include "cbor/cbor_export.h"
#include "cbor/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ============================================================================
 * Map manipulation
 * ============================================================================
 */

/** Get the number of pairs
 *
 * @param item A map
 * @return The number of pairs
 */
_CBOR_NODISCARD CBOR_EXPORT size_t cbor_map_size(const cbor_item_t* item);

/** Get the size of the allocated storage
 *
 * @param item A map
 * @return Allocated storage size (as the number of #cbor_pair items)
 */
_CBOR_NODISCARD CBOR_EXPORT size_t cbor_map_allocated(const cbor_item_t* item);

/** Create a new definite map
 *
 * @param size The number of slots to preallocate
 * @return Reference to the new map item. The item's reference count is
 * initialized to one.
 * @return `NULL` if memory allocation fails
 */
_CBOR_NODISCARD CBOR_EXPORT cbor_item_t* cbor_new_definite_map(size_t size);

/** Create a new indefinite map
 *
 * @return Reference to the new map item. The item's reference count is
 * initialized to one.
 * @return `NULL` if memory allocation fails
 */
_CBOR_NODISCARD CBOR_EXPORT cbor_item_t* cbor_new_indefinite_map(void);

/** Add a pair to the map
 *
 * For definite maps, items can only be added to the preallocated space. For
 * indefinite maps, the storage will be expanded as needed
 *
 * @param item A map
 * @param pair The key-value pair to add. Reference count of the #cbor_pair.key
 * and #cbor_pair.value will be increased by one.
 * @return `true` on success, `false` if memory allocation failed (indefinite
 * maps) or the preallocated storage is full (definite maps)
 */
_CBOR_NODISCARD CBOR_EXPORT bool cbor_map_add(cbor_item_t* item,
                                              struct cbor_pair pair);

/** Add a key to the map
 *
 * Sets the value to `NULL`. Internal API.
 *
 * @param item A map
 * @param key The key, Its reference count will be be increased by one.
 * @return `true` on success, `false` if either reallocation failed or the
 * preallocated storage is full
 */
_CBOR_NODISCARD CBOR_EXPORT bool _cbor_map_add_key(cbor_item_t* item,
                                                   cbor_item_t* key);

/** Add a value to the map
 *
 * Assumes that #_cbor_map_add_key has been called. Internal API.
 *
 * @param item A map
 * @param value The value. Its reference count will be be increased by one.
 * @return `true` on success, `false` if either reallocation failed or the
 * preallocated storage is full
 */
_CBOR_NODISCARD CBOR_EXPORT bool _cbor_map_add_value(cbor_item_t* item,
                                                     cbor_item_t* value);

/** Is this map definite?
 *
 * @param item A map
 * @return Is this map definite?
 */
_CBOR_NODISCARD CBOR_EXPORT bool cbor_map_is_definite(const cbor_item_t* item);

/** Is this map indefinite?
 *
 * @param item A map
 * @return Is this map indefinite?
 */
_CBOR_NODISCARD CBOR_EXPORT bool cbor_map_is_indefinite(
    const cbor_item_t* item);

/** Get the pairs storage
 *
 * @param item A map
 * @return Array of #cbor_map_size pairs. Manipulation is possible as long as
 * references remain valid.
 */
_CBOR_NODISCARD CBOR_EXPORT struct cbor_pair* cbor_map_handle(
    const cbor_item_t* item);

/** Look up a value in a map by key using a caller-supplied equality function
 *
 * Scans the map linearly and returns the first value whose key compares equal
 * to \p key under \p eq. This is O(n) in the number of entries.
 *
 * The equality function is intentionally parameterized. Most applications
 * constrain their map keys to a single type (e.g., text strings or small
 * integers), which lets them implement a cheaper, type-specific comparator
 * instead of a fully generic one. The parameter also allows callers to plug
 * in any desired semantics — structural equality via
 * #cbor_structurally_equal, a data-model comparator that ignores encoding
 * width, a case-insensitive string comparator, etc. — without the library
 * having to anticipate every use case.
 *
 * \rst
 * .. code-block:: c
 *
 *    // Look up a text-string key using structural equality
 *    cbor_item_t *key = cbor_build_string("alg");
 *    cbor_item_t *value = cbor_map_get(map, key, cbor_structurally_equal);
 *    if (value != NULL) {
 *        // use value ...
 *        cbor_decref(&value);
 *    }
 *    cbor_decref(&key);
 * \endrst
 *
 * @param map[borrow]  A map item; must not be `NULL`
 * @param key[borrow]  The key to search for; must not be `NULL`
 * @param eq           Equality predicate, called as `eq(candidate_key, key)`;
 *                     must not be `NULL`
 * @return[transfer] The first matching value with its reference count
 *         incremented by one, or `NULL` if no key matched. The caller is
 *         responsible for releasing the returned item with #cbor_decref.
 */
_CBOR_NODISCARD CBOR_EXPORT cbor_item_t* cbor_map_get(
    const cbor_item_t* map, const cbor_item_t* key,
    bool (*eq)(const cbor_item_t*, const cbor_item_t*));

#ifdef __cplusplus
}
#endif

#endif  // LIBCBOR_MAPS_H
