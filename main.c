#include "main.h"

List constants;
List functions;

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
			ByteMode mode = *(uint16_t*)(start);
			start += sizeof(uint16_t);
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

void run_binary(int index, char *stack, int *reg) {
	Function *function = get_function(index);
	if(!function) {
		runtime_error("Function %i not found\n", index);
	}

	char *stack_8  = (char*)stack;
	short *stack_16 = (short*)stack;
	int  *stack_32 = (int*)stack;

	int ip_save = reg[IP];
	reg[IP] = 0;

	while(reg[IP] < function->code_count) {
		OP op_row  = function->code[reg[IP]];
		ByteMode mode = op_row.mode;
		ByteCode op = op_row.op;
		int left    = op_row.left;
		int right   = op_row.right;
		switch(op) {
			case BC_PUSH: {
				if(mode & BM_L_REG) {
					*(int*)(stack_8 + reg[SP]) = reg[left];
				} else if(mode & BM_L_ADDR) {
					*(int*)(stack_8 + reg[SP]) = *(int*)(stack_8 + left);
				} else if(mode & BM_L_VAL) {
					*(int*)(stack_8 + reg[SP]) = left;
				} else {
					runtime_error("unknown bytemode access");
				}
				reg[SP] += 4;
			}
			break;
			case BC_POP: {
				reg[SP] -= 4;
				if(mode & BM_L_REG) {
					reg[left] = *(int*)(stack_8 + reg[SP]);
				} else if(mode & BM_L_ADDR) {
					*(int*)(stack_8 + left) = *(int*)(stack_8 + reg[SP]);
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
					carry = *(int*)(reg[FP] + stack_8 + right);
				} else if(mode & BM_R_VAL) {
					carry = right;
				} else {
					runtime_error("unknown bytemode access");
				}

				if(mode & BM_L_REG) {
					reg[left] = carry;
				} else if(mode & BM_L_ADDR) {
					*(int*)(reg[FP] + stack_8 + left) = carry;
				} else {
					runtime_error("unknown bytemode access");
				}
				
			}
			break;
			case BC_MOVIND: {
				// move indirect
				int carry;
				if(mode & BM_R_REG) {
					carry = *(int*)(stack_8 + reg[right]);
				} else {
					runtime_error("unknown bytemode access");
				}

				if(mode & BM_L_REG) {
					reg[left] = carry;
				} else if(mode & BM_L_ADDR) {
					*(int*)(reg[FP] + stack_8 + left) = carry;
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
			case BC_JMP: {
				if(mode & BM_L_ADDR) {
					reg[IP] = left - 1;
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_JNE: {
				if(mode & BM_L_ADDR) {
					if(reg[F_EQ] == 0) {
						reg[IP] = left - 1;
					}
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_JE: {
				if(mode & BM_L_ADDR) {
					if(reg[F_EQ] == 1) {
						reg[IP] = left - 1;
					}
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_CMP: {
				int l;
				int r;
				if(mode & BM_R_REG) {
					r = reg[right];
				} else if(mode & BM_R_ADDR) {
					r = *(int*)(reg[FP] + stack_8 + right);
				} else if(mode & BM_R_VAL) {
					r = right;
				} else {
					runtime_error("unknown bytemode access");
				}

				if(mode & BM_L_REG) {
					l = reg[left];
				} else if(mode & BM_L_ADDR) {
					l = *(int*)(reg[FP] + stack_8 + left);
				} else if(mode & BM_L_VAL) {
					l = left;
				} else {
					runtime_error("unknown bytemode access");
				}

				reg[F_GT] = 0;
				reg[F_EQ] = 0;
				reg[F_LT] = 0;

				if(l > r) {
					reg[F_GT] = 1;
				} else if(l == r) {
					reg[F_EQ] = 1;
				} else if(l < r) {
					reg[F_LT] = 1;
				}
			}
			break;
			case BC_SETEGT: {
				if(mode & BM_L_REG) {
					reg[left] = reg[F_GT];
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_SETELT: {
				if(mode & BM_L_REG) {
					reg[left] = reg[F_LT];
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_CALL: {
				if(mode & BM_L_ADDR) {
					char *name = get_from_constant_list(left);
					// printf("call: %s\n", name);
					if(strcmp(name, "dbg") == 0) {
						printf("%i\n", *(int*)(stack_8 + reg[REG_10]));
					} else {
						run_binary(left, stack, reg);
					}
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

	reg[IP] = ip_save;

	/*

	printf("-----REGS-----\n");

	for(int i = 0; i < REGSIZE; i++) {
		if(i == SP) {
			printf("SP%i: %i\n", i, reg[i]);
		} else {
			printf("REG%i: %i\n", i, reg[i]);
		}
	}

	printf("-----STACK-----\n");

	for(int i = 0; i < 128; i++) {
		if(i % 4 == 0 && i != 0) {
			printf("\n");
		}
		printf("%02x", stack[i] & 0xff);
	}
	printf("\n");

	*/
}

