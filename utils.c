#include "utils.h"

char *strmalloc(char *s) {
	size_t len = strlen(s) + 1;
	char *new_s = malloc(len);
	if(new_s == NULL) {
		runtime_error("Malloc error");
	}
	memcpy(new_s, s, len);
	return new_s;
}

void runtime_error(const char *format, ...) {
	va_list args;
	va_start(args, format);

	vprintf(format, args);

    va_end(args);
    exit(1);
}