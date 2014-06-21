#include <stdio.h>

/* http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing */
#define debug_print(fmt, ...) do { \
	if (DEBUG) \
		fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
	} while (0)

#define TO_STR_(x) #x
#define TO_STR(x) TO_STR_(x) /* enables proper double expansion */
