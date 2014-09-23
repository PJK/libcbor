#ifndef CBOR_STACK_H_
#define CBOR_STACK_H_

#include "cbor.h"

struct _cbor_stack_record {
    struct _cbor_stack_record * lower;
    cbor_type type;
};

struct _cbor_stack {
    struct _cbor_stack_record * top;
    size_t size;
};

struct _cbor_stack _cbor_stack_init();
void _cbor_stack_pop(struct _cbor_stack *);
struct _cbor_stack_record * _cbor_stack_push(struct _cbor_stack *, cbor_type);

#endif
