#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include "list.h"
#include "datatypes.h"
#include "utils.h"

typedef enum {
	DATA_ACTUAL,
	DATA_POINTER
} DataAspect;

typedef struct _StackRow {
	ListNode node;
	DataType type;
	DataType array_type;
	DataAspect aspect;
	int id;
	int size;
	int data_number;
	char *data_string;
	struct _StackRow **data_array;
	struct _StackRow *owner;
	int scope;
} StackRow;

StackRow *new_stack();
StackRow *new_stack_number(int data, int scope);
StackRow *new_stack_string(char *data, int scope);
StackRow *new_stack_array(DataType type, int size, int scope);
int stack_get_number(StackRow *stack);
char *stack_get_string(StackRow *stack);
StackRow *stack_get_array(StackRow *stack, int i);
void stack_put_array(StackRow *stack, int i, StackRow *data);
StackRow *stack_clone(StackRow *stack);
void free_stack(StackRow *stack);

#endif