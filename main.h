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
#include "sb.h"

#define STACKSIZE 65535

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
	BC_AND,
	BC_STRCONCAT,
	BC_STORE,
	BC_LOAD,
	BC_PUSH,
	BC_POP,
	BC_CALL,
	BC_RET,
	BC_CMPEQ,
	BC_CMPNEQ,
	BC_CMPGT,
	BC_CMPLT,
	BC_JNE,
	BC_JE,
	BC_JMP,
	BC_DEREF,
	BC_REF,
	BC_MOV,
	BC_MOVIND,
	BC_CMP,
	BC_SETEGT,
	BC_SETELT
} ByteCode;

typedef enum {
	BM_L = 1 << 0,
	BM_R = 1 << 1,
	BM_L_REG = 1 << 2,
	BM_R_REG = 1 << 3,
	BM_L_ADDR = 1 << 4,
	BM_R_ADDR = 1 << 5,
	BM_L_VAL = 1 << 6,
	BM_R_VAL = 1 << 7,
	BM_L_IND = 1 << 8,
	BM_R_IND = 1 << 9
} ByteMode;

typedef enum {
	REG_0  = 0,
	REG_1  = 1,
	REG_2  = 2,
	REG_3  = 3,
	REG_4  = 4,
	REG_5  = 5,
	REG_6  = 6,
	REG_7  = 7,
	REG_8  = 8,
	REG_9  = 9,
	REG_10 = 10,
	REG_11 = 11,
	REG_12 = 12,
	REG_13 = 13,
	REG_14 = 14,
	REG_15 = 15,
	SP     = 16, // stack pointer
	FP     = 17, // frame pointer
	IP     = 18, // instruction pointer
	F_GT   = 19, // flag greater
	F_EQ   = 20, // flag equal
	F_LT   = 21, // flag less
	REGSIZE
} Register;

typedef struct _ConstantPoolRow {
	ListNode node;
	unsigned index;
	char *data;
} ConstantPoolRow;

typedef struct _OP {
	ByteCode op;
	ByteMode mode;
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
void run_binary(int index, char *stack, int *reg);

#endif