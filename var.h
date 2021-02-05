#ifndef VAR_H
#define VAR_H

#include <stdlib.h>
#include "list.h"
#include "utils.h"
#include "datatypes.h"
#include "string.h"
#include "number.h"
#include "array.h"

typedef struct _VarList {
	ListNode node;
	DataType type;
	int id;
	void *data;
} VarList;

VarList *new_var(void *data, DataType type, int id);
String *var_get_string(VarList *var);
Number *var_get_number(VarList *var);
Array *var_get_array(VarList *var);
void var_move(VarList *var, VarList *new);
void free_var(VarList *var);

#endif