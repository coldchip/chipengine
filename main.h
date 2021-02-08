#ifndef MAIN_H
#define MAIN_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h> 
#include "list.h"
#include "datatypes.h"
#include "stack.h"
#include "sb.h"

#define VARSIZE 102400
#define STACKSIZE 256

typedef struct _Header {
	int magic;
	int version;
	uint64_t time;
} Header;

typedef enum {
	BC_NOP, // do nothing
	BC_NEWARRAY,
	BC_ARR_STORE,
	BC_ARR_LOAD,
	BC_ADD,
	BC_SUB,
	BC_MUL,
	BC_DIV,
	BC_SHL,
	BC_SHR,
	BC_STRCONCAT,
	BC_STORE,
	BC_LOAD,
	BC_PUSH_I, // for numbers and char
	BC_PUSH_S, // string
	BC_CALL,
	BC_RET,
	BC_CMPEQ,
	BC_CMPNEQ,
	BC_CMPGT,
	BC_CMPLT,
	BC_JMPIFEQ,
	BC_GOTO
} ByteCode; // max 64 bytecodes (00111111) 2 bits used for size


typedef struct _ConstantPoolRow {
	ListNode node;
	unsigned index;
	char *data;
} ConstantPoolRow;

typedef struct _OP {
	ByteCode op;
	int left;
	int right;
} OP;

typedef struct _Function {
	ListNode node;
	unsigned index;
	int code_count;
	OP *code;
} Function;

void load_binary();
void debug_log(const char *format, ...);
char *get_from_constant_list(int in);
int get_char_from_constant_list(char *data);
Function *get_function(unsigned index);
OP *get_op_by_index(List *code, unsigned index);
void run_binary(int index);

#endif