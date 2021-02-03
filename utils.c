#include "utils.h"

char *strmalloc(char *s) {
	size_t len = strlen(s) + 1;
	char *new_s = malloc(len * sizeof(char));
	
	memset(new_s, 0, len);
	strcpy(new_s, s);
	return new_s;
}

void runtime_error(const char *format, ...) {
	va_list args;
	va_start(args, format);

	vprintf(format, args);

    va_end(args);
    exit(1);
}