#include "string.h"

String *new_string(char *data) {
	String *string = malloc(sizeof(String));
	string->data = strmalloc(data);
	return string;
}

String *clone_string(String *old) {
	String *new = malloc(sizeof(String));
	new->data = strmalloc(old->data);
	return new;
}

void free_string(String *string) {
	free(string->data);
	free(string);
}