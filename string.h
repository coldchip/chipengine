#ifndef STRING_H
#define STRING_H

#include "utils.h"

typedef struct _String {
	char *data;
} String;

String *new_string(char *data);
String *clone_string(String *old);
void free_string(String *string);

#endif