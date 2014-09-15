#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"


static void test_simple_flags(void **state) {
	cbor_flags_t flags = { .no_realloc = true };
	assert_false(flags.canonical);
	assert_true(flags.no_realloc);
}

static void test_multiple_flags(void **state) {
	cbor_flags_t flags = { .no_realloc = true, .canonical = true };
	assert_true(flags.canonical);
	assert_true(flags.no_realloc);
}

int main(void) {
	const UnitTest tests[] = {
		unit_test(test_simple_flags),
		unit_test(test_multiple_flags)
	};
	return run_tests(tests);
}
