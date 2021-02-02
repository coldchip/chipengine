#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>
#include "list.h"
#include "string.h"
#include "number.h"
#include "char.h"
#include "datatypes.h"

typedef struct _Array {
	DataType type;
	void **data;
	int size;
} Array;

Array *new_array(int size, DataType type);
Array *clone_array(Array *old);
void put_array(Array *array, int index, void *data);
void *get_array(Array *array, int index);
void free_array(Array *array);

#endif