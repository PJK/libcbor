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
   * Subitems the records below still expect after the ones in progress,
   * saturated at `SIZE_MAX`. Set on push: only the top record receives
   * subitems.
   */
  size_t pending_below;
};

/** Subitems \p record expects after the one in progress
 *
 * Indefinite items expect none in particular and use `subitems` otherwise.
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

/** Subitems all records on \p stack expect after the ones in progress,
 * saturated at `SIZE_MAX` */
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
