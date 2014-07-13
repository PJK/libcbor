
#include <jansson.h>

#define ITERS 30000000

int main() {
    for (size_t i = 0; i < ITERS; i++) {
		json_t * i = json_integer(42);
		json_decref(i);
	}
    return 0;
}
