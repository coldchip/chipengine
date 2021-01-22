#ifndef MAIN_H
#define MAIN_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "list.h"
#include "datatypes.h"
#include "stack.h"
#include "conversion.h"
#include "var.h"

typedef enum {
	BC_PUSHSTR,
	BC_STRCONCAT,
	BC_STORE,
	BC_LOAD,
	BC_PUSH,
	BC_ADD,
	BC_SUB,
	BC_MUL,
	BC_DIV,
	BC_CALL,
	BC_RET,
	BC_GOTO
} ByteCode;

typedef struct _ConstantPoolRow {
	ListNode node;
	unsigned index;
	char *data;
} ConstantPoolRow;

typedef struct _Function {
	ListNode node;
	unsigned index;
	List code;
} Function;

typedef struct _OP {
	ListNode node;
	ByteCode op;
	int left;
	int right;
} OP;

char *strmalloc (const char *s) {
  size_t len = strlen (s) + 1;
  void *new = malloc (len);
  if (new == NULL)
    return NULL;
  return (char *) memcpy (new, s, len);
}

void load_binary();
void run_binary();

#endif