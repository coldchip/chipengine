#include "var.h"

VarList *new_var(void *data, DataType type, char *name) {
	VarList *row = malloc(sizeof(VarList));
	row->data = data;
	row->type = type;
	row->name = strmalloc(name);
	return row;
}

String *var_get_string(VarList *var) {
	return (String*)var->data;
}

Number *var_get_number(VarList *var) {
	return (Number*)var->data;
}

Array *var_get_array(VarList *var) {
	return (Array*)var->data;
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
	if(var->type == DATA_NUMBER || var->type == DATA_CHAR) {
		free_number(var->data);
	} else if(var->type == DATA_STRING) {
		free_string(var->data);
	} else if(var->type == DATA_ARRAY_MASK) {
		free_array(var->data);
	} else {
		runtime_error("var::unable to free unknown type\n");
	}
	free(var->name);
	free(var);
}