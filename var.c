#include "var.h"

VarList *new_number_var_object(int number, char *name) {
	VarList *row = malloc(sizeof(VarList));
	row->name = strmalloc(name);
	row->type = DATA_NUMBER;
	row->data_number = number;
	return row;
}

VarList *new_string_var_object(char *data, char *name) {
	VarList *row = malloc(sizeof(VarList));
	row->name = strmalloc(name);
	row->type = DATA_STRING;
	row->data_string = strmalloc(data);
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

void put_var(List *list, VarList *var) {
	VarList *is_var = get_var(list, var->name);
	if(is_var != NULL) {
		list_remove(&is_var->node);
		free_var(is_var);
	}
	list_insert(list_end(list), var);
}

void free_var(VarList *var) {
	if(var->type == DATA_STRING) {
		free(var->data_string);
	}
	free(var->name);
	free(var);
}