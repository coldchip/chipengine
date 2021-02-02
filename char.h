#ifndef CHAR_H
#define CHAR_H

#include <stdlib.h>

typedef struct _Char {
	char value;
} Char;

Char *new_char(char value);
Char *clone_char(Char *old);
void free_char(Char *c);

#endif