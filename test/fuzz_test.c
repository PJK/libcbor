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
#include <time.h>

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

unsigned seed;

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
	printf("pre - Info\n");
	printf("seed: %u\n", seed);
	printf(
		"Fuzzing %llu rounds of up to %llu bytes with seed %u\n",
		ROUNDS,
		MAXLEN,
		seed);
	printf("Info\n");
	srand(seed);
	printf("Srand\n");
	fflush(stdout);

	for (size_t i = 0; i < ROUNDS; i++) {
		run_round();
	}

	printf("Successfully fuzzed through %llu kB of data\n", (ROUNDS * MAXLEN) / 1024);
}

int main(int argc, char * argv[])
{
	printf("starting\n");
	if (argc > 1)
		seed = (unsigned)strtoul(argv[1], NULL, 10);
	else
		seed = (unsigned)time(NULL);

	printf("seed set\n");
	const UnitTest tests[] = {
		unit_test(fuzz)
	};
	return run_tests(tests);
}
