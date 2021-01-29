#include "stack.h"

StackRow *new_number_stack_object(int number) {
	StackRow *row = malloc(sizeof(StackRow));
	row->type = DATA_NUMBER;
	row->data_number = (number);
	return row;
}

StackRow *new_number_array_stack_object(int size) {
	StackRow *row = malloc(sizeof(StackRow));
	row->type = DATA_NUMBER | DATA_ARRAY_MASK;
	list_clear(&row->data_array);
	for(int i = 0; i < size; i++) {
		StackRow *elem = new_number_stack_object(0);
		list_insert(list_end(&row->data_array), elem);
	}
	return row;
}

StackRow *new_string_stack_object(char *data) {
	StackRow *row = malloc(sizeof(StackRow));
	row->type = DATA_STRING;
	row->data_string = strmalloc(data);
	return row;
}

void free_stack(StackRow *stack) {
	if(stack->type & DATA_STRING) {
		free(stack->data_string);
	}
	if(stack->type & DATA_ARRAY_MASK) {
		ListNode *elem = list_begin(&stack->data_array);
		while(elem != list_end(&stack->data_array)) {
			StackRow *r = (StackRow*)elem;
			elem = list_next(elem);
			list_remove(&r->node);
			free_stack(r);
		}
	}
	free(stack);
}