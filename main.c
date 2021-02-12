#include "main.h"

List constants;
List functions;

int transfer_size = 0;
StackRow *transfer[128];

FILE *beta = NULL;

int main(int argc, char const *argv[]) {
	/* code */
	setbuf(stdout, 0);
	load_binary();
	
	unsigned f_index = get_char_from_constant_list("main");

	int reg[REGSIZE];
	memset(reg, 0, sizeof(reg));

	char stack[STACKSIZE];
	memset(stack, 0, sizeof(stack));

	run_binary(f_index, stack, reg);

	ListNode *cs = list_begin(&constants);
	while(cs != list_end(&constants)) {
		ConstantPoolRow *con = (ConstantPoolRow*)cs;
		cs = list_next(cs);
		list_remove(&con->node);
		free(con->data);
		free(con);
	}

	ListNode *funcs = list_begin(&functions);
	while(funcs != list_end(&functions)) {
		Function *func = (Function*)funcs;
		funcs = list_next(funcs);
		list_remove(&func->node);
		free(func);
	}
	
	return 0;
}

void load_binary() {
	extern char _binary_bin_out_chip_start[];
	// extern char _binary_bin_out_bin_end[];

	char *start = _binary_bin_out_chip_start;
	// char *end = _binary_bin_out_bin_end;

	list_clear(&constants);
	list_clear(&functions);

	Header *header = (Header*)start;
	if(header->magic != 1178944383) {
		runtime_error("unable to read binary, invalid magic number\n");
	}

	if(header->version != 4) {
		runtime_error("unable to run binary, invalid version\n");
	}

	start += sizeof(Header);

	unsigned f_count = *(unsigned*)(start);
	start += sizeof(unsigned);
	for(int i = 0; i < f_count; i++) {
		unsigned f_index = *(unsigned*)(start);
		start += sizeof(unsigned);
		unsigned code_count = *(unsigned*)(start);
		start += sizeof(unsigned);

		debug_log("F %i Code Count: %i\n", f_index, code_count);
		debug_log("---------------\n");

		Function *function = malloc(sizeof(Function));
		function->code = malloc(sizeof(OP) * code_count);
		function->index = f_index;
		function->code_count = code_count;

		for(unsigned i = 0; i < code_count; i++) {
			ByteMode mode = *(uint8_t*)(start);
			start += sizeof(uint8_t);
			ByteCode op = *(uint8_t*)(start);
			start += sizeof(uint8_t);

			int left = 0;
			int right = 0;

			if(mode & BM_L) {
				left = *(int*)(start);
				start += sizeof(int);
			}

			if(mode & BM_R) {
				right = *(int*)(start);
				start += sizeof(int);
			}
			
			OP row;
			row.mode = mode;
			row.op    = op;
			row.left  = left;
			row.right = right;

			function->code[i] = row;
			
			debug_log("%i %i %i\n", op, left, right);
		}
		list_insert(list_end(&functions), function);
	}
	debug_log("\n\n");
	unsigned constant_count = *(unsigned*)(start);
	start += sizeof(unsigned);
	debug_log("Constant Count: %i\n", constant_count);
	debug_log("---------------\n");
	for(unsigned i = 0; i < constant_count; i++) {
		unsigned str_len = *(unsigned*)(start);
		start += sizeof(unsigned);

		char *data = malloc((sizeof(char) * str_len) + 1);
		memcpy(data, start, str_len);
		start += str_len;

		ConstantPoolRow *row = malloc(sizeof(ConstantPoolRow));
		row->index = i;
		row->data = data;
		list_insert(list_end(&constants), row);
		debug_log("Index: %i, Data: %s\n", i, data);
	}
}

int is_regular_file(const char *path){
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISREG(path_stat.st_mode);
}

void debug_log(const char *format, ...) {
	if(1 == 1) {
		va_list args;
		va_start(args, format);

		vprintf(format, args);
		

	    va_end(args);
	}
}

char *get_from_constant_list(int in) {
	for(ListNode *i = list_begin(&constants); i != list_end(&constants); i = list_next(i)) {
		ConstantPoolRow *row = (ConstantPoolRow*)i;
		if(row->index == in) {
			return row->data;
		}
	}
	return NULL;
}

int get_char_from_constant_list(char *data) {
	for(ListNode *i = list_begin(&constants); i != list_end(&constants); i = list_next(i)) {
		ConstantPoolRow *row = (ConstantPoolRow*)i;
		if(strcmp(row->data, data) == 0) {
			return row->index;
		}
	}
	return -1;
}

Function *get_function(unsigned index) {
	ListNode *i;
	for(i = list_begin(&functions); i != list_end(&functions); i = list_next(i)) {
		Function *row = (Function*)i;
		if(row->index == index) {
			return row;
		}
	}
	return NULL;
}

StackRow *get_var(List *list, int id) {
	for(ListNode *i = list_begin(list); i != list_end(list); i = list_next(i)) {
		StackRow *row = (StackRow*)i;
		if(row->id == id) {
			return row;
		}
	}
	return NULL;
}

void put_var(List *list, StackRow *var, int id) {
	var->id = id;
	StackRow *exists = get_var(list, id);
	if(exists) {
		list_remove(&exists->node);
		free_stack(exists);
	}
	list_insert(list_end(list), var);
	
}

void run_binary(int index, char *stack, int *reg) {
	Function *function = get_function(index);
	if(!function) {
		runtime_error("Function %i not found\n", index);
	}

	char *stack_8  = (char*)stack;
	int  *stack_32 = (int*)stack;

	while(reg[IP] < function->code_count) {
		OP op_row  = function->code[reg[IP]];
		ByteMode mode = op_row.mode;
		ByteCode op = op_row.op;
		int left    = op_row.left;
		int right   = op_row.right;
		switch(op) {
			case BC_PUSH: {
				if(mode & BM_L_REG) {
					*(stack_32 + reg[SP]) = reg[left];
				} else if(mode & BM_L_ADDR) {
					*(stack_32 + reg[SP]) = *(stack_32 + left);
				} else if(mode & BM_L_VAL) {
					*(stack_32 + reg[SP]) = left;
				} else {
					runtime_error("unknown bytemode access");
				}
				reg[SP] += 1;
			}
			break;
			case BC_POP: {
				reg[SP] -= 1;
				if(mode & BM_L_REG) {
					reg[left] = *(stack_32 + reg[SP]);
				} else if(mode & BM_L_ADDR) {
					*(stack_32 + left) = *(stack_32 + reg[SP]);
				} else {
					runtime_error("unknown bytemode access");
				}
			}
			break;
			case BC_MOV: {
				int carry;
				if(mode & BM_R_REG) {
					carry = reg[right];
				} else if(mode & BM_R_ADDR) {
					carry = *(stack_32 + right);
				} else if(mode & BM_R_VAL) {
					carry = right;
				} else {
					runtime_error("unknown bytemode access");
				}

				if(mode & BM_L_REG) {
					reg[left] = carry;
				} else if(mode & BM_L_ADDR) {
					*(stack_32 + left) = carry;
				} else {
					runtime_error("unknown bytemode access");
				}
				
			}
			break;
			case BC_ADD: {
				if(mode & BM_L_REG && mode & BM_R_REG) {
					reg[left] += reg[right];
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_SUB: {
				if(mode & BM_L_REG && mode & BM_R_REG) {
					reg[left] -= reg[right];
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_MUL: {
				if(mode & BM_L_REG && mode & BM_R_REG) {
					reg[left] *= reg[right];
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_DIV: {
				if(mode & BM_L_REG && mode & BM_R_REG) {
					reg[left] /= reg[right];
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_CALL: {
				if(mode & BM_L_ADDR) {
					char *name = get_from_constant_list(left);
					printf("call: %s\n", name);
					run_binary(left, stack, reg);
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			default: {
				runtime_error("unknown instruction: %i", op);
			}
			break;
		}
		reg[IP]++;
	}

	printf("-----REGS-----\n");

	for(int i = 0; i < REGSIZE; i++) {
		if(i == SP) {
			printf("SP%i: %i\n", i, reg[i]);
		} else {
			printf("REG%i: %i\n", i, reg[i]);
		}
	}

	printf("-----STACK-----\n");

	for(int i = 0; i < 1024; i++) {
		if(i % 4 == 0 && i != 0) {
			printf("\n");
		}
		printf("%02x", stack[i] & 0xff);
	}
	printf("\n");
}

