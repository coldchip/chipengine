#include "main.h"

List constants;
unsigned entry_point;
unsigned code_count;
OP *code;

int main(int argc, char const *argv[]) {
	/* code */
	setbuf(stdout, 0);
	load_binary();
	
	int reg[REGSIZE];
	memset(reg, 0, sizeof(reg));

	char stack[STACKSIZE];
	memset(stack, 0, sizeof(stack));

	printf("Entry Point: %u\n", entry_point);
	printf("In length: %u\n", code_count);

	run_binary(entry_point, stack, reg);

	ListNode *cs = list_begin(&constants);
	while(cs != list_end(&constants)) {
		ConstantPoolRow *con = (ConstantPoolRow*)cs;
		cs = list_next(cs);
		list_remove(&con->node);
		free(con->data);
		free(con);
	}
	
	return 0;
}

void load_binary() {
	extern char _binary_bin_out_chip_start[];
	// extern char _binary_bin_out_bin_end[];

	char *start = _binary_bin_out_chip_start;
	// char *end = _binary_bin_out_bin_end;

	list_clear(&constants);

	Header *header = (Header*)start;
	if(header->magic != 1178944383) {
		runtime_error("unable to read binary, invalid magic number\n");
	}

	if(header->version != 5) {
		runtime_error("unable to run binary, invalid version\n");
	}

	start += sizeof(Header);

	entry_point = *(unsigned*)(start);
	start += sizeof(unsigned);

	code_count = *(unsigned*)(start);
	start += sizeof(unsigned);
	
	code = malloc(sizeof(OP) * code_count);

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

		code[i] = row;
		
		debug_log("%i %i %i\n", op, left, right);
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

void debug_log(const char *format, ...) {
	if(1 == 2) {
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

void run_binary(int ip, char *stack, int *reg) {
	char *stack_8  = (char*)stack;
	short *stack_16 = (short*)stack;
	int  *stack_32 = (int*)stack;

	reg[IP] = ip;

	while(reg[IP] < code_count) {
		OP op_row  = code[reg[IP]];
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
				int carry = 0;
				if(mode & BM_R_REG) {
					carry = reg[right];
					if(mode & BM_R_IND) {
						// ptr deref
						carry = *(int*)(stack_8 + carry);
					}
				} else if(mode & BM_R_ADDR) {
					carry = *(int*)(reg[FP] + stack_8 + right);
					if(mode & BM_R_IND) {
						// ptr deref
						carry = *(int*)(stack_8 + carry);
					}
				} else if(mode & BM_R_VAL) {
					carry = right;
					if(mode & BM_R_IND) {
						// ptr deref
						carry = *(int*)(stack_8 + carry);
					}
				} else {
					runtime_error("unknown bytemode access");
				}

				if(mode & BM_L_REG) {
					if(mode & BM_L_IND) {
						// ptr deref
						int addr = reg[left];
						*(int*)(stack_8 + addr) = carry;
					} else {
						reg[left] = carry;
					}
				} else if(mode & BM_L_ADDR) {
					if(mode & BM_L_IND) {
						// ptr deref
						int addr = *(int*)(stack_8 + reg[left]);
						*(int*)(stack_8 + addr) = carry;
					} else {
						*(int*)(reg[FP] + stack_8 + left) = carry;
					}
				} else {
					runtime_error("unknown bytemode access");
				}
				
			}
			break;
			case BC_LEA: {
				int addr = 0;
				if(mode & BM_R_ADDR) {
					addr = right;
				} else {
					runtime_error("unknown bytemode access");
				}

				if(mode & BM_L_REG) {
					reg[left] = addr;
				} else if(mode & BM_L_ADDR) {
					*(int*)(reg[FP] + stack_8 + left) = addr;
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
			case BC_MOD: {
				if(mode & BM_L_REG && mode & BM_R_REG) {
					reg[left] %= reg[right];
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_AND: {
				if(mode & BM_L_REG && mode & BM_R_REG) {
					reg[left] &= reg[right];
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
				int l = 0;
				int r = 0;
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
			case BC_SETEEQ: {
				if(mode & BM_L_REG) {
					reg[left] = reg[F_EQ];
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_SETENEQ: {
				if(mode & BM_L_REG) {
					reg[left] = !reg[F_EQ];
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_RET: {
				return;
			}
			break;
			case BC_HALT: {
				return;
			}
			break;
			case BC_CALL: {
				if(mode & BM_L_ADDR) {
					int save_ip = reg[IP];
					run_binary(left, stack, reg);
					reg[IP] = save_ip;
				} else {
					runtime_error("unknown bytemode access");
				}				
			}
			break;
			case BC_SYSCALL: {
				printf("%i\n", *(int*)(stack_8 + reg[REG_10]));
			}
			break;
			default: {
				runtime_error("unknown instruction: %i", op);
			}
			break;
		}
		reg[IP]++;
	}
}

