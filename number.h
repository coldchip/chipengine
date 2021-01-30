#ifndef NUMBER_H
#define NUMBER_H

#include <stdlib.h>

typedef struct _Number {
	int value;
} Number;

Number *new_number(int value);
Number *clone_number(Number *old);
void free_number(Number *number);

#endif