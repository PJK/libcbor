/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBCBOR_TAGS_H
#define LIBCBOR_TAGS_H

#include <stdint.h>

#include "cbor/cbor_export.h"
#include "cbor/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ============================================================================
 * Standard tag numbers (RFC 8949)
 * ============================================================================
 */

/*
 * Scope: Only the 15 tag numbers in RFC 8949 Section 3.4, Table 5 (STD 94)
 * belong here. Tag 35 (regular expressions, RFC 7049) is excluded because
 * RFC 8949 omitted it from Table 5. Do not add entries from the broader IANA
 * CBOR Tags registry; protocol-specific tag numbers and any validation of
 * tagged content belong in application code.
 *
 * Rationale for `UINT64_C` macros (rather than `enum`): CBOR tag numbers form
 * an open 64-bit space (0..2^64-1) and all tag APIs accept and return
 * `uint64_t`, whereas C99 enumerators are restricted to `int`. Defining these
 * constants via `UINT64_C` gives them the exact `uint64_t` width (avoiding
 * format-string, signedness, and 16-bit `int` pitfalls) and avoids introducing
 * an enum type that could silently narrow arbitrary 64-bit tag values.
 */

/** @name Standard tag numbers (RFC 8949 Section 3.4, Table 5)
 *
 * Only the core tags from RFC 8949 Table 5 (STD 94) are defined here; tags
 * registered in the broader IANA CBOR Tags registry are left to applications.
 * Tag functions (#cbor_new_tag, #cbor_build_tag, #cbor_tag_value,
 * #cbor_encode_tag) accept and return any `uint64_t` tag value, not just these
 * constants.
 *
 * These constants are numeric identifiers only: libcbor does not interpret or
 * validate the type or format of the tagged item.
 * @{
 */

/** Tag 0: Standard date/time string (RFC 3339 text string; RFC 8949
 * Section 3.4.1). */
#define CBOR_TAG_DATETIME_STRING UINT64_C(0)

/** Tag 1: Epoch-based date/time (integer or float seconds since
 * 1970-01-01T00:00Z; RFC 8949 Section 3.4.2). */
#define CBOR_TAG_DATETIME_EPOCH UINT64_C(1)

/** Tag 2: Unsigned bignum (byte string; RFC 8949 Section 3.4.3). */
#define CBOR_TAG_UNSIGNED_BIGNUM UINT64_C(2)

/** Tag 3: Negative bignum (byte string; RFC 8949 Section 3.4.3). */
#define CBOR_TAG_NEGATIVE_BIGNUM UINT64_C(3)

/** Tag 4: Decimal fraction (`[exponent, mantissa]` array; RFC 8949
 * Section 3.4.4). */
#define CBOR_TAG_DECIMAL_FRACTION UINT64_C(4)

/** Tag 5: Bigfloat (`[exponent, mantissa]` array; RFC 8949 Section 3.4.4). */
#define CBOR_TAG_BIGFLOAT UINT64_C(5)

/** Tag 21: Expected conversion to base64url encoding (RFC 8949
 * Section 3.4.5.2). */
#define CBOR_TAG_EXPECTED_BASE64URL UINT64_C(21)

/** Tag 22: Expected conversion to base64 encoding (RFC 8949 Section 3.4.5.2).
 */
#define CBOR_TAG_EXPECTED_BASE64 UINT64_C(22)

/** Tag 23: Expected conversion to base16 encoding (RFC 8949 Section 3.4.5.2).
 */
#define CBOR_TAG_EXPECTED_BASE16 UINT64_C(23)

/** Tag 24: Encoded CBOR data item (byte string; RFC 8949 Section 3.4.5.1). */
#define CBOR_TAG_ENCODED_CBOR UINT64_C(24)

/** Tag 32: URI (RFC 3986 text string; RFC 8949 Section 3.4.5.3). */
#define CBOR_TAG_URI UINT64_C(32)

/** Tag 33: base64url-encoded text (RFC 4648 text string; RFC 8949
 * Section 3.4.5.3). */
#define CBOR_TAG_BASE64URL UINT64_C(33)

/** Tag 34: base64-encoded text (RFC 4648 text string; RFC 8949
 * Section 3.4.5.3). */
#define CBOR_TAG_BASE64 UINT64_C(34)

/** Tag 36: MIME message (RFC 2045 text string; RFC 8949 Section 3.4.5.3). */
#define CBOR_TAG_MIME_MESSAGE UINT64_C(36)

/** Tag 55799: Self-described CBOR (`0xd9d9f7` prefix; RFC 8949 Section 3.4.6).
 */
#define CBOR_TAG_SELF_DESCRIBED_CBOR UINT64_C(55799)

/** @} */

/*
 * ============================================================================
 * Tag manipulation
 * ============================================================================
 */

/** Create a new tag.
 *
 * @param value The tag value (number); see the `CBOR_TAG_*` constants (e.g.
 * #CBOR_TAG_DATETIME_STRING) for RFC 8949 tags, or pass any 64-bit tag number.
 * @return Reference to the new tag. Its reference count is initialized to one
 * and it points to a `NULL` item.
 * @return `NULL` if memory allocation fails.
 */
_CBOR_NODISCARD CBOR_EXPORT cbor_item_t* cbor_new_tag(uint64_t value);

/** Get the tagged item (what the tag points to).
 *
 * @param tag A #CBOR_TYPE_TAG tag.
 * @return Reference to the tagged item, which may be `NULL` if no item has
 * been set yet.
 *
 * If non-NULL, increases the reference count of the underlying item. The
 * returned reference must be released using #cbor_decref.
 */
_CBOR_NODISCARD CBOR_EXPORT cbor_item_t* cbor_tag_item(const cbor_item_t* tag);

/** Get the tag value.
 *
 * @param tag A #CBOR_TYPE_TAG tag.
 * @return The tag value (number); see the `CBOR_TAG_*` constants (e.g.
 * #CBOR_TAG_DATETIME_STRING) for RFC 8949 tags.
 */
_CBOR_NODISCARD CBOR_EXPORT uint64_t cbor_tag_value(const cbor_item_t* tag);

/** Assign a tag to an item.
 *
 * @param tag A #CBOR_TYPE_TAG tag.
 * @param tagged_item The item to tag. Its reference count will be increased
 * by one.
 *
 * If the tag already points to an item, the previous item's reference count
 * will be decreased by one.
 */
CBOR_EXPORT void cbor_tag_set_item(cbor_item_t* tag, cbor_item_t* tagged_item);

/** Build a new tag.
 *
 * @param item The item to tag. Its reference count will be increased by
 * one.
 * @param value The tag value (number); see the `CBOR_TAG_*` constants (e.g.
 * #CBOR_TAG_DATETIME_STRING) for RFC 8949 tags, or pass any 64-bit tag number.
 * @return Reference to the new tag item. The item's reference count is
 * initialized to one.
 * @return `NULL` if memory allocation fails.
 */
_CBOR_NODISCARD CBOR_EXPORT cbor_item_t* cbor_build_tag(uint64_t value,
                                                        cbor_item_t* item);

#ifdef __cplusplus
}
#endif

#endif  // LIBCBOR_TAGS_H
