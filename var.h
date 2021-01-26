#ifndef VAR_H
#define VAR_H

#include <stdlib.h>
#include "list.h"
#include "utils.h"
#include "datatypes.h"

typedef struct _VarList {
	ListNode node;
	DataType type;
	char *name;
	int data_number;
	char *data_string;
} VarList;

VarList *new_number_var_object(int number, char *name);
VarList *new_string_var_object(char *data, char *name);
VarList *get_var(List *list, char *name);
void put_var(List *varlist, VarList *var);
void free_var(VarList *var);

#endif