/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include "cbor.h"

/* Verify that CBOR_TAG_* macros are uint64_t constant expressions. */
#ifdef LIBCBOR_TAGS_H
#if CBOR_TAG_DATETIME_STRING != 0 || CBOR_TAG_SELF_DESCRIBED_CBOR != 55799
#error "Unexpected CBOR_TAG_* value in preprocessor expression"
#endif
#endif

typedef char check_tag_zero[CBOR_TAG_DATETIME_STRING == 0 ? 1 : -1];
typedef char check_tag_self[CBOR_TAG_SELF_DESCRIBED_CBOR == 55799 ? 1 : -1];
typedef char
    check_tag_width[sizeof(CBOR_TAG_DATETIME_STRING) == sizeof(uint64_t) ? 1
                                                                         : -1];

int main() {
  uint64_t tag = CBOR_TAG_SELF_DESCRIBED_CBOR;
  switch (tag) {
    case CBOR_TAG_SELF_DESCRIBED_CBOR:
      std::cout << "Hello from libcbor " << CBOR_VERSION << std::endl;
      return 0;
    default:
      return 1;
  }
}
