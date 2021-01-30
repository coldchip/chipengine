#include "number.h"

Number *new_number(int value) {
	Number *number = malloc(sizeof(Number));
	number->value = value;
	return number;
}

Number *clone_number(Number *old) {
	Number *new = malloc(sizeof(Number));
	new->value = old->value;
	return new;
}

void free_number(Number *number) {
	free(number);
}