#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include "list.h"
#include "datatypes.h"
#include "utils.h"

typedef struct _StackRow {
	ListNode node;
	DataType type;
	int data_number;
	char *data_string;
	List data_array;
} StackRow;

StackRow *new_number_stack_object(int number);
StackRow *new_number_array_stack_object(int size);
StackRow *new_string_stack_object(char *data);
void free_stack(StackRow *stack);

#endif