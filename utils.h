#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

char *strmalloc(char *s);
void runtime_error(const char *format, ...);

#endif