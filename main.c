#include "main.h"

List constants;
List functions;

int transfer_size = 0;
StackRow *transfer[1024];

FILE *beta = NULL;

int main(int argc, char const *argv[]) {
	/* code */
	load_binary();
	
	unsigned f_index = get_char_from_constant_list("main");
	run_binary(f_index);

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

	if(header->version != 2) {
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
			ByteCode op = *(uint8_t*)(start);
			start += sizeof(uint8_t);
			int left = *(int*)(start);
			start += sizeof(int);
			int right = *(int*)(start);
			start += sizeof(int);
			

			OP row;
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
	if(1 == 12) {
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

VarList *get_var(List *list, int id) {
	ListNode *i;
	for(i = list_begin(list); i != list_end(list); i = list_next(i)) {
		VarList *row = (VarList*)i;
		if(row->id == id) {
			return row;
		}
	}
	return NULL;
}

void put_var(List *list, VarList *var) {
	VarList *exists = get_var(list, var->id);
	if(exists) {
		var_move(exists, var);
	} else {
		list_insert(list_end(list), var);
	}
}

void run_binary(int index) {
	Function *function = get_function(index);
	if(!function) {
		runtime_error("Function %i not found\n", index);
	}

	int sp = 0;

	StackRow *stack[STACKSIZE];
	
	List varlist;
	list_clear(&varlist);

	while(transfer_size > 0) {
		StackRow *pop = transfer[--transfer_size];
		stack[sp++] = pop;
	}

	int ip = 0;
	while(ip < function->code_count) {
		OP op_row  = function->code[ip];
		ByteCode op = op_row.op;
		int left    = op_row.left;
		int right   = op_row.right;
		switch(op) {
			case BC_NEWARRAY: {
				StackRow *size = stack[--sp];
				if(size->type == DATA_NUMBER) {
					stack[sp++] = new_stack(new_array(stack_get_number(size)->value, left), DATA_ARRAY_MASK);
				} else {
					runtime_error("newarray requires a length stackobject on stack\n");
				}
				free_stack(size);
				debug_log("newarray %i\n", left);
			}
			break;
			
			case BC_ARR_STORE: {
				StackRow *index = stack[--sp];
				StackRow *value = stack[--sp];
				if(index->type == DATA_NUMBER) {
					VarList *var = get_var(&varlist, left);
					if(var->type == DATA_ARRAY_MASK) {
						Array *array = var_get_array(var);
						put_array(array, stack_get_number(index)->value, value->data);
					} else {
						runtime_error("storing data inside a non array type isn't allowed");
					}
				} else {
					runtime_error("arr_store requires a index stackobject on stack");
				}
				free_stack(index);
				free_stack(value);
				debug_log("arr_store %i\n", left);
			}
			break;
			case BC_ARR_LOAD: {
				StackRow *index = stack[--sp];
				if(index->type == DATA_NUMBER) {
					VarList *var = get_var(&varlist, left);
					if(var->type == DATA_ARRAY_MASK) {
						Array *array = var_get_array(var);
						void *da = get_array(array, stack_get_number(index)->value);
						if(array->type == DATA_CHAR) {
							array->type = DATA_NUMBER;
						}
						stack[sp++] = new_stack(da, array->type);
					} else {
						runtime_error("loading data inside a non array type isn't allowed");
					}
				} else {
					runtime_error("arr_load requires a index stackobject on stack");
				}
				free_stack(index);
				debug_log("arr_load %i\n", left);
			}
			break;
			case BC_PUSH_I: {
				// push type number
				stack[sp++] = new_stack(new_number(left), DATA_NUMBER);
				debug_log("push_i %i\n", left);
			}
			break;
			case BC_PUSH_S: {
				// push type string
				char *string = get_from_constant_list(left);
				stack[sp++] = new_stack(new_string(string), DATA_STRING);
				debug_log("push_s %s\n", string);
			}
			break;
			case BC_STORE: {
				StackRow *pop = stack[--sp];

				VarList *var = varobject_from_stackobject(pop, left);
				put_var(&varlist, var);
				debug_log("store %i\n", left);
				
				free_stack(pop);
			}
			break;
			case BC_LOAD: {
				VarList *var = get_var(&varlist, left);
				if(!var) {
					runtime_error("Load var failed\n");
				}

				stack[sp++] = stackobject_from_varobject(var);

				debug_log("load %i\n", var->id);
			}
			break;
			case BC_STRCONCAT: {
				// mem issue
				StackRow *first = stack[--sp];
				StackRow *last  = stack[--sp];
				if(first->type == DATA_STRING && last->type == DATA_STRING) {
					// string + string
					char buf[strlen(stack_get_string(first)->data) + strlen(stack_get_string(last)->data) + 1];
					memset(buf, 0, sizeof(buf)); // clear buffer
					strcpy(buf, stack_get_string(first)->data);
					strcat(buf, stack_get_string(last)->data);

					stack[sp++] = new_stack(new_string(buf), DATA_STRING);
				} else if(first->type == DATA_STRING && last->type == DATA_NUMBER) {
					// string + char
					char buf[strlen(stack_get_string(first)->data) + 1 + 1]; // strlen(string) + sizeof(char) + 1
					memset(buf, 0, sizeof(buf)); // clear buffer
					strcpy(buf, stack_get_string(first)->data);
					char append = (char)stack_get_number(last)->value;
					strncat(buf, &append, 1);

					stack[sp++] = new_stack(new_string(buf), DATA_STRING);
				} else {
					runtime_error("concat requires 2 string type\n");
				}
				free_stack(first);
				free_stack(last);
				debug_log("strconcat\n");
			}
			break;
			case BC_ADD: {
				StackRow *pop  = stack[--sp];
				StackRow *back = stack[sp - 1];
				stack_get_number(back)->value = stack_get_number(pop)->value + stack_get_number(back)->value;

				free_stack(pop);
				debug_log("add\n");
			}
			break;
			case BC_SUB: {
				StackRow *pop  = stack[--sp];
				StackRow *back = stack[sp - 1];
				stack_get_number(back)->value = stack_get_number(pop)->value - stack_get_number(back)->value;

				free_stack(pop);
				debug_log("sub\n");
			}
			break;
			case BC_MUL: {
				StackRow *pop  = stack[--sp];
				StackRow *back = stack[sp - 1];
				stack_get_number(back)->value = stack_get_number(pop)->value * stack_get_number(back)->value;

				free_stack(pop);
				debug_log("mul\n");
			}
			break;
			case BC_DIV: {
				StackRow *pop  = stack[--sp];
				StackRow *back = stack[sp - 1];
				stack_get_number(back)->value = stack_get_number(pop)->value / stack_get_number(back)->value;

				free_stack(pop);
				debug_log("div\n");
			}
			break;
			case BC_CMPEQ: {
				StackRow *pop  = stack[--sp];
				StackRow *pop2 = stack[--sp];
				if(pop->type == DATA_NUMBER && pop2->type == DATA_NUMBER) {
					if(stack_get_number(pop)->value == stack_get_number(pop2)->value) {
						stack[sp++] = new_stack(new_number(1), DATA_NUMBER);
					} else {
						stack[sp++] = new_stack(new_number(0), DATA_NUMBER);
					}
				} else {
					runtime_error("comparison in cmpeq requires 2 number data type");
				}
				free_stack(pop);
				free_stack(pop2);
				debug_log("cmpeq\n");
			}
			break;
			case BC_CMPNEQ: {
				StackRow *pop  = stack[--sp];
				StackRow *pop2 = stack[--sp];
				if(pop->type == DATA_NUMBER && pop2->type == DATA_NUMBER) {
					if(stack_get_number(pop)->value != stack_get_number(pop2)->value) {
						stack[sp++] = new_stack(new_number(1), DATA_NUMBER);
					} else {
						stack[sp++] = new_stack(new_number(0), DATA_NUMBER);
					}
				} else {
					runtime_error("comparison in cmpneq requires 2 number data type");
				}
				free_stack(pop);
				free_stack(pop2);
				debug_log("cmpneq\n");
			}
			break;
			case BC_CMPGT: {
				StackRow *pop  = stack[--sp];
				StackRow *pop2 = stack[--sp];
				if(pop->type == DATA_NUMBER && pop2->type == DATA_NUMBER) {
					if(stack_get_number(pop)->value > stack_get_number(pop2)->value) {
						stack[sp++] = new_stack(new_number(1), DATA_NUMBER);
					} else {
						stack[sp++] = new_stack(new_number(0), DATA_NUMBER);
					}
				} else {
					runtime_error("comparison in cmpgt requires 2 number data type");
				}
				free_stack(pop);
				free_stack(pop2);
				debug_log("cmpgt\n");
			}
			break;
			case BC_CMPLT: {
				StackRow *pop  = stack[--sp];
				StackRow *pop2 = stack[--sp];
				if(pop->type == DATA_NUMBER && pop2->type == DATA_NUMBER) {
					if(stack_get_number(pop)->value < stack_get_number(pop2)->value) {
						stack[sp++] = new_stack(new_number(1), DATA_NUMBER);
					} else {
						stack[sp++] = new_stack(new_number(0), DATA_NUMBER);
					}
				} else {
					runtime_error("comparison in cmplt requires 2 number data type");
				}
				free_stack(pop);
				free_stack(pop2);
				debug_log("cmplt\n");
			}
			break;
			case BC_JMPIFEQ: {
				StackRow *pop  = stack[--sp];
				StackRow *pop2  = stack[--sp];
				if(pop->type == DATA_NUMBER && pop2->type == DATA_NUMBER) {
					if(stack_get_number(pop)->value == stack_get_number(pop2)->value) {
						ip = left-1;
					}
				} else {
					runtime_error("comparison in jmpifeq requires 2 number data type");
				}
				free_stack(pop);
				free_stack(pop2);
				debug_log("jmpifeq\n");
			}
			break;
			case BC_RET: {
				StackRow *pop = stack[--sp];
				transfer[transfer_size++] = pop;
				debug_log("ret\n");
				goto release;
			}
			break;
			case BC_GOTO: {
				ip = left-1;
				debug_log("goto\n");
			}
			break;
			case BC_CALL: {
				char *name = get_from_constant_list(left);
				debug_log("call %s\n", name);
				if(name) {
					StackRow *count = stack[--sp];
					if(strcmp(name, "__callinternal__printf") == 0) {
						StackRow *pop = stack[--sp];
						if(pop->type == DATA_NUMBER) {
							printf("%i", stack_get_number(pop)->value);
						} else if(pop->type == DATA_STRING) {
							printf("%s", stack_get_string(pop)->data);
						} else if(pop->type == DATA_CHAR) {
							printf("%c", (char)stack_get_number(pop)->value);
						} else {
							runtime_error("unable to print unsupported type\n");
						}
						free_stack(pop);
					} else if(strcmp(name, "dbgstack") == 0) {
						printf("-----DBGSTACK-----\n");
						for(int x = 0; x < sp; x++) {
							StackRow *row = stack[x];
							printf("----------\n");
							printf("data_int %i\n", stack_get_number(row)->value);
							printf("----------\n");
						}
					} else if(strcmp(name, "dbgvars") == 0) {
						/*
						for(int i = 0; i < VARSIZE; i++) {
							VarList *row = varlist[i];
							printf("----------\n");
							printf("%s data_int %i\n", row->name, var_get_number(row)->value);
							printf("----------\n");
						}
						*/
					} else if(strcmp(name, "__callinternal__strlen") == 0) {
						StackRow *str = stack[--sp];
						if(str->type == DATA_STRING) {
							int str_len = strlen(stack_get_string(str)->data);
							
							stack[sp++] = new_stack(new_number(str_len), DATA_NUMBER);
						} else {
							runtime_error("invalid type passed to __callinternal__strlen");
						}
						free_stack(str);
					} else if(strcmp(name, "__callinternal__charat") == 0) {
						StackRow *index = stack[--sp];
						StackRow *str = stack[--sp];
						if(str->type == DATA_STRING && index->type == DATA_NUMBER) {
							char strat = *(stack_get_string(str)->data + stack_get_number(index)->value);
							stack[sp++] = new_stack(new_number(strat), DATA_NUMBER);
						} else {
							runtime_error("invalid type passed to __callinternal__charat");
						}
						free_stack(index);
						free_stack(str);
					} else if(strcmp(name, "__callinternal__new_socket") == 0) {
						int fd = socket(AF_INET, SOCK_STREAM, 0);
						if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {

						}
						stack[sp++] = new_stack(new_number(fd), DATA_NUMBER);
					} else if(strcmp(name, "__callinternal__socket_bind") == 0) {
						StackRow *port = stack[--sp];
						StackRow *ip = stack[--sp];
						StackRow *fd = stack[--sp];
						if(fd->type == DATA_NUMBER && ip->type == DATA_STRING && port->type == DATA_NUMBER) {
							struct sockaddr_in addr;
							addr.sin_family = AF_INET; 
							addr.sin_addr.s_addr = inet_addr(stack_get_string(ip)->data); 
							addr.sin_port = htons(stack_get_number(port)->value); 
							if((bind(stack_get_number(fd)->value, (struct sockaddr*)&addr, sizeof(addr))) == 0 && (listen(stack_get_number(fd)->value, 5)) == 0) {
								stack[sp++] = new_stack(new_number(1), DATA_NUMBER);
							} else {
								stack[sp++] = new_stack(new_number(0), DATA_NUMBER);
							}
						} else {
							runtime_error("invalid type passed to __callinternal__socket_bind");
						}
						free_stack(port);
						free_stack(ip);
						free_stack(fd);
					} else if(strcmp(name, "__callinternal__socket_accept") == 0) {
						StackRow *fd = stack[--sp];
						if(fd->type == DATA_NUMBER) {
							struct sockaddr_in addr;
							socklen_t addr_len = sizeof(addr);
							int client = accept(stack_get_number(fd)->value, (struct sockaddr*)&addr, &addr_len); 
							stack[sp++] = new_stack(new_number(client), DATA_NUMBER);
						} else {
							runtime_error("invalid type passed to __callinternal__socket_bind");
						}
						free_stack(fd);
					} else if(strcmp(name, "__callinternal__socket_read") == 0) {
						StackRow *size = stack[--sp];
						StackRow *fd = stack[--sp];
						if(fd->type == DATA_NUMBER && size->type == DATA_NUMBER) {
							char buf[stack_get_number(size)->value];
							int size = read(stack_get_number(fd)->value, buf, sizeof(buf));

							Array *arr = new_array(size, DATA_CHAR);
							for(int i = 0; i < size; i++) {
								Number *t = new_number((int)buf[i]);
								put_array(arr, i, t);
								free_number(t);
							}

							stack[sp++] = new_stack(arr, DATA_ARRAY_MASK);
						} else {
							runtime_error("invalid type passed to __callinternal__socket_bind");
						}
						free_stack(size);
						free_stack(fd);
					} else if(strcmp(name, "__callinternal__socket_write") == 0) {
						StackRow *data = stack[--sp];
						StackRow *fd = stack[--sp];
						if(fd->type == DATA_NUMBER && data->type == DATA_ARRAY_MASK) {
							Array *data_arr = stack_get_array(data);
							char result[data_arr->size];
							for(int i = 0; i < sizeof(result); i++) {
								Number *t = (Number*)get_array(data_arr, i);
								result[i] = (char)t->value;
								free_number(t);
							}
							int s = write(stack_get_number(fd)->value, result, sizeof(result));
							
							stack[sp++] = new_stack(new_number(s), DATA_NUMBER);
						} else {
							runtime_error("invalid type passed to __callinternal__socket_bind");
						}
						free_stack(data);
						free_stack(fd);
					} else if(strcmp(name, "__callinternal__socket_close") == 0) {
						StackRow *fd = stack[--sp];
						if(fd->type == DATA_NUMBER) {
							close(stack_get_number(fd)->value);
						} else {
							runtime_error("invalid type passed to __callinternal__socket_bind");
						}
						free_stack(fd);
					} else if(strcmp(name, "__callinternal__itos") == 0) {
						StackRow *num = stack[--sp];
						if(num->type == DATA_NUMBER) {
							char buf[33];
							sprintf(buf, "%i", stack_get_number(num)->value);
							stack[sp++] = new_stack(new_string(buf), DATA_STRING);
						} else {
							runtime_error("invalid type passed to __callinternal__itos");
						}
						free_stack(num);
					} else if(strcmp(name, "__callinternal__rand") == 0) {
						StackRow *min = stack[--sp];
						StackRow *max = stack[--sp];
						if(min->type == DATA_NUMBER && max->type == DATA_NUMBER) {
							int randnum = stack_get_number(min)->value + rand() / (RAND_MAX / (stack_get_number(max)->value - stack_get_number(min)->value + 1) + 1);
							stack[sp++] = new_stack(new_number(randnum), DATA_NUMBER);
						} else {
							runtime_error("invalid type passed to __callinternal__rand");
						}
						free_stack(min);
						free_stack(max);
					} else if(strcmp(name, "__callinternal__sizeof") == 0) {
						StackRow *arr = stack[--sp];
						if(arr->type == DATA_ARRAY_MASK) {
							int size = stack_get_array(arr)->size;
							stack[sp++] = new_stack(new_number(size), DATA_NUMBER);
						} else {
							runtime_error("invalid type passed to __callinternal__sizeof");
						}
						free_stack(arr);
					} else if(strcmp(name, "__callinternal__char_to_str_cast") == 0) {
						StackRow *num = stack[--sp];
						if(num->type == DATA_CHAR) {
							char data[2];
							data[1] = '\0';
							data[0] = stack_get_number(num)->value;
							stack[sp++] = new_stack(new_string(data), DATA_STRING);
						} else if(num->type == DATA_NUMBER) {
							char data[2];
							data[1] = '\0';
							data[0] = (char)stack_get_number(num)->value;
							stack[sp++] = new_stack(new_string(data), DATA_STRING);
						} else {
							runtime_error("invalid type passed to __callinternal__char_to_str_cast");
						}
						free_stack(num);
					} else if(strcmp(name, "__callinternal__char_to_int_cast") == 0) {
						StackRow *num = stack[--sp];
						if(num->type == DATA_CHAR) {
							int c = stack_get_number(num)->value;
							stack[sp++] = new_stack(new_number(c), DATA_NUMBER);
						} else {
							runtime_error("invalid type passed to __callinternal__char_to_int_cast");
						}
						free_stack(num);
					} else if(strcmp(name, "__callinternal__exec") == 0) {
						StackRow *cmd = stack[--sp];
						if(cmd->type == DATA_STRING) {
							FILE *fp = popen(stack_get_string(cmd)->data, "r");

							if(!fp) {
								runtime_error("unable to run command");
							}
							StringBuilder *sb = sb_create();
							char buffer[128];
							while(fgets(buffer, sizeof(buffer), fp) > 0) {
								sb_append(sb, buffer);
							}
							pclose(fp);
							char *res = sb_concat(sb);
							sb_free(sb);

							stack[sp++] = new_stack(new_string(res), DATA_STRING);

							free(res);
						} else {
							runtime_error("invalid type passed to __callinternal__itos");
						}
						free_stack(cmd);
					} else if(strcmp(name, "__callinternal__fopen") == 0) {
						StackRow *mode = stack[--sp];
						StackRow *file = stack[--sp];
						if(file->type == DATA_STRING && mode->type == DATA_STRING) {
							if(is_regular_file(stack_get_string(file)->data)) {
								beta = fopen(stack_get_string(file)->data, stack_get_string(mode)->data);
								if(!beta) {
									stack[sp++] = new_stack(new_number(0), DATA_NUMBER);
								} else {
									stack[sp++] = new_stack(new_number(1), DATA_NUMBER);
								}
							} else {
								stack[sp++] = new_stack(new_number(0), DATA_NUMBER);
							}
						} else {
							runtime_error("invalid type passed to __callinternal__fopen");
						}
						free_stack(mode);
						free_stack(file);
					} else if(strcmp(name, "__callinternal__fread") == 0) {
						StackRow *size = stack[--sp];
						StackRow *fp = stack[--sp];
						if(fp->type == DATA_NUMBER && size->type == DATA_NUMBER) {
							char buf[stack_get_number(size)->value];
							int r = fread(buf, sizeof(char), stack_get_number(size)->value, beta);

							Array *arr = new_array(r, DATA_CHAR);
							for(int i = 0; i < r; i++) {
								Number *t = new_number((int)buf[i]);
								put_array(arr, i, t);
								free_number(t);
							}

							stack[sp++] = new_stack(arr, DATA_ARRAY_MASK);
						} else {
							runtime_error("invalid type passed to __callinternal__fopen");
						}
						free_stack(fp);
						free_stack(size);
					} else if(strcmp(name, "__callinternal__fclose") == 0) {
						StackRow *fp = stack[--sp];
						if(fp->type == DATA_NUMBER) {
							fclose(beta);
						} else {
							runtime_error("invalid type passed to __callinternal__fopen");
						}
						free_stack(fp);
					} else {
						//clock_t begin = clock();

						if(count && count->type != DATA_NUMBER) {
							runtime_error("Opps, unable to figure number of args to pass to stack\n");
						}
						for(int s = 0; s < stack_get_number(count)->value; s++) {
							StackRow *pop = stack[--sp];
							transfer[transfer_size++] = pop;
						}

						run_binary(left);

						while(transfer_size > 0) {
							stack[sp++] = transfer[--transfer_size];
						}

						//clock_t end = clock();

						//double time_elapsed = (double)(end - begin) / CLOCKS_PER_SEC;

						//if(time_elapsed > 0.001) {
					    	//printf("\n%s took %f seconds to execute \n", get_from_constant_list(left), time_elapsed); 
						//}
					}
					free_stack(count);
				} else {
					runtime_error("Unable get function name from constant list");
				}

			}
			break;
			default: {
				runtime_error("unknown instruction: %i", op);
			}
			break;
		}
		ip++; // ip
	}

	release:;

	ListNode *c = list_begin(&varlist);
	while(c != list_end(&varlist)) {
		VarList *row = (VarList*)c;
		c = list_next(c);
		list_remove(&row->node);
		free_var(row);

	}

	for(int h = 0; h < sp; h++) {
		free_stack(stack[h]);
	}

	
}

