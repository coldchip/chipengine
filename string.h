#ifndef STRING_H
#define STRING_H

#include "utils.h"

typedef struct _String {
	char *data;
} String;

String *new_string(char *data);
String *clone_string(String *old);
void concat_string(String *string, char *a, char *b);
void free_string(String *string);

#endif