#include "conversion.h"

// Converts a stack object to a variable
VarList *varobject_from_stackobject(StackRow *stackrow, char *name) {
	switch(stackrow->type) {
		case DATA_STRING: {
			VarList *obj = new_string_var_object(stackrow->data_string, name);
			return obj;
		}
		break;
		case DATA_NUMBER: {
			VarList *obj = new_number_var_object(stackrow->data_number, name);
			return obj;
		}
		break;
		default: {
			runtime_error("Unable to convert stack object to variable object");
		}
		break;
	}
	return NULL;
}

StackRow *stackobject_from_varobject(VarList *varobj) {
	StackRow *row = malloc(sizeof(StackRow));
	switch(varobj->type) {
		case DATA_STRING: {
			StackRow *obj = new_string_stack_object(varobj->data_string);
			return obj;
		}
		break;
		case DATA_NUMBER: {
			StackRow *obj = new_number_stack_object(varobj->data_number);
			return obj;
		}
		break;
		default: {
			runtime_error("Unable to convert stack object to variable object");
		}
		break;
	}
	return NULL;
}