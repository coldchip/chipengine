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