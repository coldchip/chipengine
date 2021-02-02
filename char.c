#include "char.h"

Char *new_char(char value) {
	Char *c = malloc(sizeof(Char));
	c->value = value;
	return c;
}

Char *clone_char(Char *old) {
	Char *new = malloc(sizeof(Char));
	new->value = old->value;
	return new;
}

void free_char(Char *c) {
	free(c);
}