/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "cbor.h"
#include <inttypes.h>
#include <stdlib.h>

#ifdef HUGE_FUZZ

#define ROUNDS 65536ULL
#define MAXLEN 131072ULL

#else

#define ROUNDS 256ULL
#define MAXLEN 2048ULL

#endif

#ifdef CBOR_PRINT_FUZZ
static void printmem(const unsigned char * ptr, size_t length)
{
	for (size_t i = 0; i < length; i++)
		printf("%02X", ptr[i]);
	printf("\n");
}
#endif

static void run_round()
{
	cbor_item_t *item;
	struct cbor_load_result res;

	size_t length = rand() % MAXLEN + 1;
	unsigned char * data = malloc(length);
	for (size_t i = 0; i < length; i++) {
		data[i] = rand() % 0xFF;
	}

	#ifdef CBOR_PRINT_FUZZ
	printmem(data, length);
	#endif

	item = cbor_load(data, length, &res);

	if (res.error.code == CBOR_ERR_NONE)
		cbor_decref(&item);
	/* Otherwise there should be nothing left behind by the decoder */

	free(data);
}

static void fuzz(void **state)
{
	int seed = rand();
	printf(
		"Fuzzing %llu rounds of up to %llu bytes with seed %d\n",
		ROUNDS,
		MAXLEN,
		seed);
	srand(seed);

	for (size_t i = 0; i < ROUNDS; i++)
		run_round();

	printf("Successfully fuzzed through %llu MB of data\n", (ROUNDS * MAXLEN) / (1024 * 1024));
}

int main(void)
{
	const UnitTest tests[] = {
		unit_test(fuzz)
	};
	return run_tests(tests);
}
