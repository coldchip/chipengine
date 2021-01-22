#include "stack.h"

StackRow *new_number_stack_object(int number) {
	StackRow *row = malloc(sizeof(StackRow));
	row->type = DATA_NUMBER;
	row->data_number = number;
	return row;
}

StackRow *new_string_stack_object(char *data) {
	StackRow *row = malloc(sizeof(StackRow));
	row->type = DATA_STRING;
	row->data_string = data;
	return row;
}