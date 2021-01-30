#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include "list.h"
#include "datatypes.h"
#include "utils.h"
#include "string.h"
#include "number.h"
#include "array.h"

typedef struct _StackRow {
	ListNode node;
	DataType type;
	void *data;
} StackRow;

StackRow *new_stack(void *data, DataType type);
String *stack_get_string(StackRow *stack);
Number *stack_get_number(StackRow *stack);
void free_stack(StackRow *stack);

#endif