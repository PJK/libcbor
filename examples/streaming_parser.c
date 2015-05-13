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
#include <stdio.h>
#include <string.h>

/*
 * Illustrates how one might skim through a map (which is assumed to have
 * string keys and values only), looking for the value of a specific key
 *
 * Use the examples/data/map.cbor input to test this.
 */

const char * key = "a secret key";
bool key_found = false;

void find_string(void * _ctx, cbor_data buffer, size_t len)
{
	if (key_found) {
		printf("Found the value: %*s\n", (int) len, buffer);
		key_found = false;
	} else if (len == strlen(key)) {
		key_found = (memcmp(key, buffer, len) == 0);
	}
}

int main(int argc, char * argv[])
{
	int fd = open(argv[1], O_RDONLY);
	struct stat info;
	fstat(fd, &info);
	unsigned char * buffer = mmap(NULL, info.st_size, PROT_READ,
	                              MAP_PRIVATE, fd, 0);

	struct cbor_callbacks callbacks = cbor_empty_callbacks;
	struct cbor_decoder_result decode_result;
	size_t bytes_read = 0;
	callbacks.string = find_string;
	while (bytes_read < info.st_size) {
		decode_result = cbor_stream_decode(buffer + bytes_read,
		                                   info.st_size - bytes_read,
		                                   &callbacks, NULL);
		bytes_read += decode_result.read;
	}
}

