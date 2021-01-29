#include "conversion.h"

// Converts a stack object to a variable
VarList *varobject_from_stackobject(StackRow *stackrow, char *name) {
	if(stackrow->type & DATA_STRING) {
		if(stackrow->type & DATA_ARRAY_MASK) {
			runtime_error("in progress");
		} else {
			VarList *obj = new_string_var_object(stackrow->data_string, name);
			return obj;
		}
	} else if(stackrow->type & DATA_NUMBER) {
		if(stackrow->type & DATA_ARRAY_MASK) {
			VarList *obj = new_number_array_var_object(list_size(&stackrow->data_array), name);
			ListNode *src = list_begin(&stackrow->data_array);
			ListNode *dest = list_begin(&obj->data_array);
			while(dest != list_end(&obj->data_array)) {
				VarList *dst = (VarList*)dest;
				dst->data_number = ((StackRow*)src)->data_number;
				src = list_next(src);
				dest = list_next(dest);
			}
			return obj;
		} else {
			VarList *obj = new_number_var_object(stackrow->data_number, name);
			return obj;
		}
	} else {
		runtime_error("unknown type to convert");
	}
	return NULL;
}

StackRow *stackobject_from_varobject(VarList *varobj) {
	if(varobj->type & DATA_STRING) {
		if(varobj->type & DATA_ARRAY_MASK) {
			runtime_error("in progress");
		} else {
			StackRow *obj = new_string_stack_object(varobj->data_string);
			return obj;
		}
	} else if(varobj->type & DATA_NUMBER) {
		if(varobj->type & DATA_ARRAY_MASK) {
			StackRow *obj = new_number_array_stack_object(list_size(&varobj->data_array));
			
			ListNode *src = list_begin(&varobj->data_array);
			ListNode *dest = list_begin(&obj->data_array);
			while(dest != list_end(&obj->data_array)) {
				StackRow *dst = (StackRow*)dest;
				dst->data_number = ((VarList*)src)->data_number;
				src = list_next(src);
				dest = list_next(dest);
			}
			return obj;
		} else {
			StackRow *obj = new_number_stack_object(varobj->data_number);
			return obj;
		}
	} else {
		runtime_error("unknown type to convert");
	}
	return NULL;
}