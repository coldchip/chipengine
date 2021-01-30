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

void concat_string(String *string, char *a, char *b) {
	char *new = malloc(strlen(a) + strlen(b) + 1);
	strcpy(new, a);
	strcat(new, b);
	free(string->data);
	string->data = new;
}

void free_string(String *string) {
	free(string->data);
	free(string);
}