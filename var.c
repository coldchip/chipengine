#include "var.h"

VarList *new_number_var_object(int number, char *name) {
	VarList *row = malloc(sizeof(VarList));
	row->name = name;
	row->type = DATA_NUMBER;
	row->data_number = number;
	return row;
}

VarList *new_string_var_object(char *data, char *name) {
	VarList *row = malloc(sizeof(VarList));
	row->name = name;
	row->type = DATA_STRING;
	row->data_string = data;
	return row;
}

VarList *get_var(List *list, char *name) {
	for(ListNode *i = list_begin(list); i != list_end(list); i = list_next(i)) {
		VarList *row = (VarList*)i;
		if(strcmp(row->name, name) == 0) {
			return row;
		}
	}
	return NULL;
}