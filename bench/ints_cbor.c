#include "cbor.h"

#define ITERS 30000000

int main() {
	for (size_t i = 0; i < ITERS; i++) {
		cbor_item_t * i = cbor_new_int8();
		cbor_set_uint8(i, 42);
		cbor_decref(&i);
	}
	return 0;
}
