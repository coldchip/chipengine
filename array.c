#include "array.h"

Array *new_array(int size, DataType type) {
	Array *arr = malloc(sizeof(Array));
	arr->type = type; // Of course we don't need array mask here because it's already defined at parent
	arr->data = (void**)malloc(sizeof(void**) * size);
	arr->size = size;
	for(int i = 0; i < size; i++) {
		if(type == DATA_NUMBER || type == DATA_CHAR) {
			Number *number = new_number(0);
			arr->data[i] = number;
		} else if(type == DATA_STRING) {
			String *string = new_string("");
			arr->data[i] = string;
		} else {
			runtime_error("unable to create array with unknown type");
		}
	}
	return arr;
}

Array *clone_array(Array *old) {
	Array *new = malloc(sizeof(Array));
	new->type = old->type;
	new->data = (void**)malloc(sizeof(void**) * old->size);
	new->size = old->size;
	for(int i = 0; i < old->size; i++) {
		if(old->type == DATA_NUMBER || old->type == DATA_CHAR) {
			new->data[i] = clone_number(old->data[i]);
		} else if(old->type == DATA_STRING) {
			new->data[i] = clone_string(old->data[i]);
		} else if(old->type == DATA_ARRAY_MASK) {
			new->data[i] = clone_array(old->data[i]);
		} else {
			runtime_error("unable to store array, unknown type\n");
		}
	}
	return new;
}

void put_array(Array *array, int index, void *data) {
	if(array->type == DATA_NUMBER || array->type == DATA_CHAR) {
		free_number(array->data[index]);
	} else if(array->type == DATA_STRING) {
		free_string(array->data[index]);
	} else if(array->type == DATA_ARRAY_MASK) {
		free_array(array->data[index]);
	} else {
		runtime_error("unable to free array element, unknown type\n");
	}

	if(array->type == DATA_NUMBER || array->type == DATA_CHAR) {
		array->data[index] = clone_number(data);
	} else if(array->type == DATA_STRING) {
		array->data[index] = clone_string(data);
	} else if(array->type == DATA_ARRAY_MASK) {
		array->data[index] = clone_array(data);
	} else {
		runtime_error("unable to store array, unknown type\n");
	}
}

void *get_array(Array *array, int index) {
	if(array->type == DATA_NUMBER || array->type == DATA_CHAR) {
		return clone_number(array->data[index]);
	} else if(array->type == DATA_STRING) {
		return clone_string(array->data[index]);
	} else if(array->type == DATA_ARRAY_MASK) {
		return clone_array(array->data[index]);
	} else {
		runtime_error("unable to store array, unknown type\n");
	}
	return NULL;
}

void free_array(Array *array) {
	for(int i = 0; i < array->size; i++) {
		if(array->type == DATA_NUMBER || array->type == DATA_CHAR) {
			free_number(array->data[i]);
		} else if(array->type == DATA_STRING) {
			free_string(array->data[i]);
		} else if(array->type == DATA_ARRAY_MASK) {
			free_array(array->data[i]);
		} else {
			runtime_error("unable to free unknown type\n");
		}
	}
	free(array->data);
	free(array);
}