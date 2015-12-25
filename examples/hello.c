/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "cbor.h"
#include <stdio.h>

int main(int argc, char * argv[])
{
	printf("Hello from libcbor %s\n", CBOR_VERSION);
	printf("Custom allocation support: %s\n", CBOR_CUSTOM_ALLOC ? "yes" : "no");
	printf("Buffer growth factor: %f\n", CBOR_BUFFER_GROWTH);
}
