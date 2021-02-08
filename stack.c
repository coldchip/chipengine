#include "stack.h"

StackRow *new_stack() {
	StackRow *stack = malloc(sizeof(StackRow));
	return stack;
}

StackRow *new_stack_number(int data, int scope) {
	StackRow *stack = malloc(sizeof(StackRow));
	stack->data_number = data;
	stack->type = DATA_NUMBER;
	stack->aspect = DATA_ACTUAL;
	stack->scope = scope;
	return stack;
}

StackRow *new_stack_string(char *data, int scope) {
	StackRow *stack = malloc(sizeof(StackRow));
	stack->data_string = strmalloc(data);
	stack->type = DATA_STRING;
	stack->aspect = DATA_ACTUAL;
	stack->scope = scope;
	return stack;
}

StackRow *new_stack_array(DataType type, int size, int scope) {
	StackRow *stack = malloc(sizeof(StackRow));
	stack->data_array = (StackRow**)malloc(sizeof(StackRow**) * size);
	stack->type = DATA_ARRAY_MASK;
	stack->aspect = DATA_ACTUAL;
	stack->array_type = type;
	stack->size = size;
	stack->scope = scope;
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
		return new_stack_number(stack->data_number, stack->scope);
	} else if(stack->type == DATA_STRING) {
		StackRow *row = new_stack();
		row->data_string = stack->data_string;
		row->type = DATA_STRING;
		row->aspect = DATA_POINTER;
		row->owner = stack;
		row->scope = stack->scope;
		return row;
	} else if(stack->type == DATA_ARRAY_MASK) {
		StackRow *row = new_stack();
		row->data_array = stack->data_array;
		row->size = stack->size;
		row->type = DATA_ARRAY_MASK;
		row->array_type = stack->array_type;
		row->aspect = DATA_POINTER;
		row->owner = stack;
		row->scope = stack->scope;
		return row;
	} else {
		runtime_error("unable to clone");
	}
	return NULL;
}

void free_stack(StackRow *stack) {
	if(stack->aspect == DATA_ACTUAL) {
		if(stack->type == DATA_STRING) {
			free(stack->data_string);
		}
		if(stack->type == DATA_ARRAY_MASK) {
			for(int i = 0; i < stack->size; i++) {
				free_stack(stack->data_array[i]);
			}
			free(stack->data_array);
		}
	}
	free(stack);
}