#ifndef CONVERSION_H
#define CONVERSION_H

#include "stack.h"
#include "var.h"

VarList *varobject_from_stackobject(StackRow *stackrow, char *name);
StackRow *stackobject_from_varobject(VarList *varobj);

#endif