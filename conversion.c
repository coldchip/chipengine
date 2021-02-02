#include "conversion.h"

// Converts a stack object to a variable
VarList *varobject_from_stackobject(StackRow *stack, char *name) {
	if(stack->type & DATA_STRING) {
		return new_var(clone_string(stack->data), DATA_STRING, name);
	} else if(stack->type & DATA_NUMBER) {
		return new_var(clone_number(stack->data), DATA_NUMBER, name);
	} else if(stack->type & DATA_CHAR) {
		return new_var(clone_char(stack->data), DATA_CHAR, name);
	} else if(stack->type & DATA_ARRAY_MASK) {
		return new_var(clone_array(stack->data), DATA_ARRAY_MASK, name);
	} else {
		runtime_error("unknown type to convert\n");
	}
	return NULL;
}

StackRow *stackobject_from_varobject(VarList *var) {
	if(var->type & DATA_STRING) {
		return new_stack(clone_string(var->data), DATA_STRING);
	} else if(var->type & DATA_NUMBER) {
		return new_stack(clone_number(var->data), DATA_NUMBER);
	} else if(var->type & DATA_CHAR) {
		return new_stack(clone_char(var->data), DATA_CHAR);
	} else if(var->type & DATA_ARRAY_MASK) {
		return new_stack(clone_array(var->data), DATA_ARRAY_MASK);
	} else {
		runtime_error("unknown type to convert\n");
	}
	return NULL;
}