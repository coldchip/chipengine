#ifndef DATATYPES_H
#define DATATYPES_H

typedef enum {
	DATA_STRING = 1 << 0,
	DATA_NUMBER = 2 << 0,
	DATA_CHAR = 3 << 0,
	DATA_VOID = 4 << 0,
	DATA_ARRAY_MASK = 1 << 5
} DataType;

#endif