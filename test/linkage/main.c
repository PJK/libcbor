/*
 * Copyright (c) 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

/*
 * Standalone linkage test: verifies that libcbor can be found via
 * find_package and linked against from an external project, for both
 * static and shared library builds.
 */

#include <assert.h>
#include <stdio.h>
#include "cbor.h"

int main(void) {
  /* Encode an integer and decode it back */
  cbor_item_t* item = cbor_build_uint8(42);
  assert(item != NULL);

  unsigned char buf[16];
  size_t len = cbor_serialize(item, buf, sizeof(buf));
  assert(len > 0);

  struct cbor_load_result result;
  cbor_item_t* decoded = cbor_load(buf, len, &result);
  assert(decoded != NULL);
  assert(result.error.code == CBOR_ERR_NONE);
  assert(cbor_get_uint8(decoded) == 42);

  cbor_decref(&item);
  cbor_decref(&decoded);

  printf("libcbor linkage test passed (version %s)\n", CBOR_VERSION);
  return 0;
}
