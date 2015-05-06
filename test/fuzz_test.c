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

#define ROUNDS 16000
#define MAXLEN 6

static void printmem(const unsigned char * ptr, size_t length)
{
	for (size_t i = 0; i < length; i++)
		printf("%02X", ptr[i]);
	printf("\n");
}

static void run_round()
{
	cbor_item_t *item;
	struct cbor_load_result res;

	size_t length = rand() % MAXLEN + 1;
	unsigned char * data = malloc(length);
	for (size_t i = 0; i < length; i++)
		data[i] = rand() % 0xFF;

	printmem(data, length);

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
		"Fuzzing %d rounds of up to %d bytes with seed %d\n",
		ROUNDS,
		MAXLEN,
		seed);
	srand(seed);

	for (size_t i = 0; i < ROUNDS; i++)
		run_round();
}

int main(void)
{
	const UnitTest tests[] = {
		unit_test(fuzz)
	};
	return run_tests(tests);
}
