/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBCBOR_STACK_H
#define LIBCBOR_STACK_H

#include "cbor/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Simple stack record for the parser */
struct _cbor_stack_record {
  /** Pointer to the parent stack frame */
  struct _cbor_stack_record* lower;
  /** Item under construction */
  cbor_item_t* item;
  /**
   * How many outstanding subitems are expected.
   *
   * For example, when we see a new definite array, `subitems` is initialized to
   * the array length. With every item added, the counter is decreased. When it
   * reaches zero, the stack is popped and the complete item is propagated
   * upwards.
   */
  size_t subitems;
  /**
   * How many subitems the records below this one still expect, not counting
   * the one each of them has in progress (see #_cbor_stack_record_pending).
   * Saturates at `SIZE_MAX`.
   *
   * Only the top record receives subitems, so the records below do not change
   * while this one is on the stack.
   */
  size_t pending_below;
};

/** How many subitems \p record expects after the one in progress
 *
 * For a definite array or map, that is one less than `subitems`. Tags expect
 * one subitem, which is the one in progress. Indefinite items expect no
 * particular count and use `subitems` for other purposes.
 */
static inline size_t _cbor_stack_record_pending(
    const struct _cbor_stack_record* record) {
  return record->subitems > 0 ? record->subitems - 1 : 0;
}

/** Stack handle - contents and size */
struct _cbor_stack {
  struct _cbor_stack_record* top;
  size_t size;
};

/** How many subitems all records on \p stack still expect, not counting the
 * ones in progress
 *
 * Saturates at `SIZE_MAX`.
 */
static inline size_t _cbor_stack_pending(const struct _cbor_stack* stack) {
  if (stack->top == NULL) return 0;
  size_t below = stack->top->pending_below;
  size_t own = _cbor_stack_record_pending(stack->top);
  return own > SIZE_MAX - below ? SIZE_MAX : below + own;
}

_CBOR_NODISCARD
struct _cbor_stack _cbor_stack_init(void);

void _cbor_stack_pop(struct _cbor_stack*);

_CBOR_NODISCARD
struct _cbor_stack_record* _cbor_stack_push(struct _cbor_stack*, cbor_item_t*,
                                            size_t);

#ifdef __cplusplus
}
#endif

#endif  // LIBCBOR_STACK_H
