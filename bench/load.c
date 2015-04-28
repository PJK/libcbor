/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "cbor.h"
#include "../src/cbor/data.h"
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>


static struct timeval tm1;

static inline void start()
{
	gettimeofday(&tm1, NULL);
}

static inline void stop()
{
	struct timeval tm2;
	gettimeofday(&tm2, NULL);

	unsigned long long t = 1000 * (tm2.tv_sec - tm1.tv_sec) + (tm2.tv_usec - tm1.tv_usec) / 1000;
	printf("%llu ms\n", t);
}

#define ITERS 30000000

int main(int argc, char * argv[]) {
	struct stat sb;
	int fd = open(argv[1], O_RDONLY);
	fstat(fd, &sb);
	printf("Size: %lu\n", (uint64_t)sb.st_size);

	char * memblock = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	start();
	json_t * item = json_loadb(memblock, sb.st_size, 0, NULL);
	stop();

	int fd2 = open(argv[2], O_RDONLY);
	fstat(fd2, &sb);
	printf("Size: %lu\n", (uint64_t)sb.st_size);

	char * memblock2 = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd2, 0);

	struct cbor_load_result res;
	start();
	cbor_item_t * it = cbor_load(memblock2, sb.st_size, &res);
	stop();
	return 0;
}

