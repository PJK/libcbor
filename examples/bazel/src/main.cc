#include "src/hello.h"

#include <stdio.h>

int main() {
  printf("Hello, v=%d\n", cbor_version());
  return 0;
}

