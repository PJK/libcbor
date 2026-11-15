/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#include "cbor/internal/stack.h"
#include "assertions.h"
#include "cbor.h"

static void test_pending(void** _state _CBOR_UNUSED) {
  struct _cbor_stack stack = _cbor_stack_init();
  /* Records do not look at their items */
  cbor_item_t* item = cbor_build_uint8(0);
  assert_size_equal(_cbor_stack_pending(&stack), 0);

  /* A definite array with three items left, one of them in progress */
  assert_non_null(_cbor_stack_push(&stack, item, 3));
  assert_size_equal(stack.top->pending_below, 0);
  assert_size_equal(_cbor_stack_pending(&stack), 2);

  /* A definite map with two pairs left */
  assert_non_null(_cbor_stack_push(&stack, item, 4));
  assert_size_equal(stack.top->pending_below, 2);
  assert_size_equal(_cbor_stack_pending(&stack), 5);

  /* Items arrive at the top record only */
  stack.top->subitems--;
  assert_size_equal(_cbor_stack_pending(&stack), 4);

  /* A tag: its only subitem is the one in progress */
  assert_non_null(_cbor_stack_push(&stack, item, 1));
  assert_size_equal(stack.top->pending_below, 4);
  assert_size_equal(_cbor_stack_pending(&stack), 4);

  /* An indefinite map flips between expecting a key and a value */
  assert_non_null(_cbor_stack_push(&stack, item, 0));
  assert_size_equal(_cbor_stack_pending(&stack), 4);
  stack.top->subitems ^= 1;
  assert_size_equal(_cbor_stack_pending(&stack), 4);

  _cbor_stack_pop(&stack);
  _cbor_stack_pop(&stack);
  assert_size_equal(_cbor_stack_pending(&stack), 4);
  _cbor_stack_pop(&stack);
  _cbor_stack_pop(&stack);
  assert_size_equal(_cbor_stack_pending(&stack), 0);
  cbor_decref(&item);
}

static void test_pending_saturates(void** _state _CBOR_UNUSED) {
  struct _cbor_stack stack = _cbor_stack_init();
  cbor_item_t* item = cbor_build_uint8(0);
  assert_non_null(_cbor_stack_push(&stack, item, SIZE_MAX));
  assert_size_equal(_cbor_stack_pending(&stack), SIZE_MAX - 1);
  assert_non_null(_cbor_stack_push(&stack, item, 3));
  assert_size_equal(stack.top->pending_below, SIZE_MAX - 1);
  assert_size_equal(_cbor_stack_pending(&stack), SIZE_MAX);
  assert_non_null(_cbor_stack_push(&stack, item, 3));
  assert_size_equal(stack.top->pending_below, SIZE_MAX);
  assert_size_equal(_cbor_stack_pending(&stack), SIZE_MAX);
  _cbor_stack_pop(&stack);
  _cbor_stack_pop(&stack);
  _cbor_stack_pop(&stack);
  cbor_decref(&item);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_pending),
      cmocka_unit_test(test_pending_saturates),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
