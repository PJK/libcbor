/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "cbor.h"

int main(void) {
  printf("Hello from libcbor %s\n", CBOR_VERSION);
  printf("Pretty-printer support: %s\n", CBOR_PRETTY_PRINTER ? "yes" : "no");
  printf("Buffer growth factor: %f\n", (float)CBOR_BUFFER_GROWTH);
}
