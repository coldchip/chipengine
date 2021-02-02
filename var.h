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
	char *name;
	void *data;
} VarList;

VarList *new_var(void *data, DataType type, char *name);
String *var_get_string(VarList *var);
Number *var_get_number(VarList *var);
Char *var_get_char(VarList *var);
Array *var_get_array(VarList *var);
VarList *get_var(List *list, char *name);
void put_var(List *varlist, VarList *var);
void free_var(VarList *var);

#endif