/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBCBOR_MAP_UTILS_H
#define LIBCBOR_MAP_UTILS_H

#include "cbor/cbor_export.h"
#include "cbor/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Incremental map construction, used by #cbor_map_add and the decoder. The
 * caller must pair every #_cbor_map_add_key with an #_cbor_map_add_value
 * before the map is used otherwise. Not exported from the shared library.
 */

/** Add a key to the map
 *
 * Sets the value to `NULL`.
 *
 * @param item A map
 * @param key The key. Its reference count will be increased by one.
 * @return `true` on success, `false` if either reallocation failed or the
 * preallocated storage is full
 */
_CBOR_NODISCARD CBOR_NO_EXPORT bool _cbor_map_add_key(cbor_item_t* item,
                                                      cbor_item_t* key);

/** Add a value to the map
 *
 * Sets the value of the key added by the immediately preceding
 * #_cbor_map_add_key call.
 *
 * @param item A map whose last key has no value yet
 * @param value The value. Its reference count will be increased by one.
 * @return Always `true`
 */
_CBOR_NODISCARD CBOR_NO_EXPORT bool _cbor_map_add_value(cbor_item_t* item,
                                                        cbor_item_t* value);

#ifdef __cplusplus
}
#endif

#endif  // LIBCBOR_MAP_UTILS_H
