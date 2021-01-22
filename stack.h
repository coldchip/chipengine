#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include "list.h"
#include "datatypes.h"

typedef struct _StackRow {
	ListNode node;
	DataType type;
	int data_number;
	char *data_string;
} StackRow;

StackRow *new_number_stack_object(int number);
StackRow *new_string_stack_object(char *data);

#endif