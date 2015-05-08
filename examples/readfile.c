/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "cbor.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char * argv[])
{
	int fd = open(argv[1], O_RDONLY);
	struct stat info;
	fstat(fd, &info);
	unsigned char * buffer = mmap(NULL, info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	/* Assuming `buffer` contains `info.st_size` bytes of input data */
	struct cbor_load_result result;
	cbor_item_t * item = cbor_load(buffer, info.st_size, &result);
	/* Pretty-print the result */
	cbor_describe(item, stdout);
	fflush(stdout);
	/* Deallocate the result */
	cbor_decref(&item);

	munmap(buffer, info.st_size);
	close(fd);
}
