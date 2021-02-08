#include "stack.h"

StackRow *new_stack_number(int data) {
	StackRow *stack = malloc(sizeof(StackRow));
	stack->data_number = data;
	stack->type = DATA_NUMBER;
	return stack;
}

StackRow *new_stack_string(char *data) {
	StackRow *stack = malloc(sizeof(StackRow));
	stack->data_string = strmalloc(data);
	stack->type = DATA_STRING;
	return stack;
}

StackRow *new_stack_array(DataType type, int size) {
	StackRow *stack = malloc(sizeof(StackRow));
	stack->data_array = (StackRow**)malloc(sizeof(StackRow**) * size);
	stack->type = DATA_ARRAY_MASK;
	stack->array_type = type;
	stack->size = size;
	for(int i = 0; i < size; i++) {
		stack->data_array[i] = NULL;
	}
	return stack;
}

int stack_get_number(StackRow *stack) {
	return stack->data_number;
}

char *stack_get_string(StackRow *stack) {
	return stack->data_string;
}

StackRow *stack_get_array(StackRow *stack, int i) {
	if(stack->type == DATA_ARRAY_MASK) {
		return stack_clone(stack->data_array[i]);
	} else {
		runtime_error("stack_get_array not an array");
	}
	return NULL;
}

void stack_put_array(StackRow *stack, int i, StackRow *data) {
	if(stack->type == DATA_ARRAY_MASK) {
		stack->data_array[i] = stack_clone(data);
	} else {
		runtime_error("stack_get_array not an array");
	}
}

StackRow *stack_clone(StackRow *stack) {
	if(stack->type == DATA_NUMBER) {
		return new_stack_number(stack->data_number);
	} else if(stack->type == DATA_STRING) {
		return new_stack_string(stack->data_string);
	} else if(stack->type == DATA_ARRAY_MASK) {
		StackRow *row = new_stack_array(stack->array_type, stack->size);
		for(int i = 0; i < stack->size; i++) {
			row->data_array[i] = stack_clone(stack->data_array[i]);
		}
		return row;
	} else {
		runtime_error("unable to clone");
	}
	return NULL;
}

void free_stack(StackRow *stack) {
	if(stack->type == DATA_STRING) {
		free(stack->data_string);
	}
	if(stack->type == DATA_ARRAY_MASK) {
		for(int i = 0; i < stack->size; i++) {
			free_stack(stack->data_array[i]);
		}
		free(stack->data_array);
	}
	free(stack);
}