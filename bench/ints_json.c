/*
 * Copyright (c) 2009-2013 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <string.h>

#include <jansson.h>

#define ITERS 30000000

int main() {
    for (size_t i = 0; i < ITERS; i++) {
		json_t * i = json_integer(42);
		json_decref(i);
	}
    return 0;
}
