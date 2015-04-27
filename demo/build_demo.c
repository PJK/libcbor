/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <cbor.h>
#include <stdio.h>

cbor_item_t * number;
struct cbor_load_result res;

unsigned char data[] = { 0x02 };
unsigned char data2[] = { 0x18, 0xFF };
unsigned char data3[] = { 0x19, 0x01, 0xf4 };

int main() {
	number = cbor_load(data, 1, &res);
	printf("Hello world, this is libcbor v%s\n", CBOR_VERSION);
	printf("Result: %u\n", res.error.code);
	printf("Result: %d, %hhu\n", cbor_typeof(number), cbor_get_uint8(number));

	number = cbor_load(data2, 2, &res);
	printf("Result: %u\n", res.error.code);
	printf("Result: %d, %hhu\n", cbor_typeof(number), cbor_get_uint8(number));

	number = cbor_load(data3, 3, &res);
	printf("Result: %u\n", res.error.code);
	printf("Result: %d, %u\n", cbor_typeof(number), cbor_get_uint16(number));
}
