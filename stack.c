#include "stack.h"

StackRow *new_stack(void *data, DataType type) {
	StackRow *stack = malloc(sizeof(StackRow));
	stack->data = data;
	stack->type = type;
	return stack;
}

String *stack_get_string(StackRow *stack) {
	return (String*)stack->data;
}

Number *stack_get_number(StackRow *stack) {
	return (Number*)stack->data;
}

void free_stack(StackRow *stack) {
	if(stack->type == DATA_NUMBER) {
		free_number(stack->data);
	} else if(stack->type == DATA_STRING) {
		free_string(stack->data);
	} else if(stack->type == DATA_ARRAY_MASK) {
		free_array(stack->data);
	} else {
		runtime_error("unable to free unknown type\n");
	}
	free(stack);
}