/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "cbor.h"
//#include <cbor.h>

#define ITERS 30000000

int main() {
	for (size_t i = 0; i < ITERS; i++) {
		cbor_item_t * i = cbor_new_int8();
		cbor_set_uint8(i, 42);
		cbor_decref(&i);
	}
	return 0;
}
