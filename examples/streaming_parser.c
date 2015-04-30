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

const char * key = "a secret key";
bool key_found = false;

enum cbor_callback_result find_string(void * _ctx, cbor_data buffer, size_t len)
{
	if (key_found)
		printf("Found the value: %*s\n", (int) len, buffer);
	else if (len == strlen(key))
		key_found = memcmp(key, buffer, len);
	return CBOR_CALLBACK_OK;
}

int main(int argc, char * argv[])
{
	int fd = open(argv[1], O_RDONLY);
	struct stat info;
	fstat(fd, &info);
	unsigned char * buffer = mmap(NULL, info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	struct cbor_callbacks callbacks = cbor_empty_callbacks;
	struct cbor_decoder_result decode_result;
	size_t bytes_read = 0;
	callbacks.string = find_string;
	while (bytes_read < info.st_size) {
		decode_result = cbor_stream_decode(buffer, info.st_size - bytes_read, &callbacks, NULL);
		bytes_read += decode_result.read;
	}
}

