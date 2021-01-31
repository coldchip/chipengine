#include "main.h"

List constants;
List functions;
List transfer;

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
		ListNode *o = list_begin(&func->code);
		while(o != list_end(&func->code)) {
			OP *y = (OP*)o;
			o = list_next(o);
			list_remove(&func->node);
			free(y);
		}
		free(func);
	}
	
	return 0;
}

void load_binary() {
	extern char _binary_bin_out_bin_start[];
	extern char _binary_bin_out_bin_end[];

	char *start = _binary_bin_out_bin_start;
	char *end = _binary_bin_out_bin_end;

	list_clear(&constants);
	list_clear(&functions);
	list_clear(&transfer);

	start += 8;

	unsigned f_count = *(unsigned*)(start);
	start += sizeof(unsigned);
	for(int i = 0; i < f_count; i++) {
		unsigned f_index = *(unsigned*)(start);
		start += sizeof(unsigned);
		unsigned code_count = *(unsigned*)(start);
		start += sizeof(unsigned);

		debug_log("Code Count: %i\n", code_count);
		debug_log("---------------\n");

		Function *function = malloc(sizeof(Function));
		list_clear(&function->code);
		function->index = f_index;

		for(unsigned i = 0; i < code_count; i++) {
			ByteCode op = *(uint8_t*)(start);
			start += sizeof(uint8_t);
			int left = *(int*)(start);
			start += sizeof(int);
			int right = *(int*)(start);
			start += sizeof(int);
			

			OP *row = malloc(sizeof(OP));
			row->op = op;
			row->left = left;
			row->right = right;

			list_insert(list_end(&function->code), row);
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

void debug_log(const char *format, ...) {
	va_list args;
	va_start(args, format);

	if(1 == 12) {
		vprintf(format, args);
	}

    va_end(args);
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
	for(ListNode *i = list_begin(&functions); i != list_end(&functions); i = list_next(i)) {
		Function *row = (Function*)i;
		if(row->index == index) {
			return row;
		}
	}
	return NULL;
}

OP *get_op_by_index(List *code, unsigned index) {
	int n = 0;
	for(ListNode *i = list_begin(code); i != list_end(code); i = list_next(i)) {
		OP *row = (OP*)i;
		if(n == index) {
			return row;
		}
		n++;
	}
	runtime_error("Unable to get op from index");
	return NULL;
}

void run_binary(int index) {
	Function *function = get_function(index);
	if(!function) {
		runtime_error("Function %i not found\n", index);
	}

	List stack;
	list_clear(&stack);
	List varlist;
	list_clear(&varlist);

	while(list_size(&transfer) > 0) {
		StackRow *pop = (StackRow*)list_remove(list_back(&transfer));
		list_insert(list_end(&stack), pop);
	}

	ListNode *i = list_begin(&function->code);
	while(i != list_end(&function->code)) {
		OP *op_row  = (OP*)i;
		ByteCode op = op_row->op;
		int left    = op_row->left;
		int right   = op_row->right;
		switch(op) {
			case BC_NEWARRAY: {
				StackRow *size = (StackRow*)list_remove(list_back(&stack));
				if(size->type == DATA_NUMBER) {
					if(left == DATA_NUMBER) {
						StackRow *array = new_stack(new_array(stack_get_number(size)->value, left), DATA_ARRAY_MASK);
						list_insert(list_end(&stack), array);
					} else if(left == DATA_STRING) {
						StackRow *array = new_stack(new_array(stack_get_number(size)->value, left), DATA_ARRAY_MASK);
						list_insert(list_end(&stack), array);
					} else {
						runtime_error("unable to create unknown type newarray\n");
					}
				} else {
					runtime_error("newarray requires a length stackobject on stack\n");
				}
				free_stack(size);
				debug_log("newarray %i\n", left);
			}
			break;
			
			case BC_ARR_STORE: {
				StackRow *index = (StackRow*)list_remove(list_back(&stack));
				StackRow *value = (StackRow*)list_remove(list_back(&stack));
				if(index->type == DATA_NUMBER) {
					char *name = get_from_constant_list(left);
					VarList *var = get_var(&varlist, name);
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
				StackRow *index = (StackRow*)list_remove(list_back(&stack));
				if(index->type == DATA_NUMBER) {
					char *name = get_from_constant_list(left);
					VarList *var = get_var(&varlist, name);
					if(var->type == DATA_ARRAY_MASK) {
						Array *array = var_get_array(var);
						void *da = get_array(array, stack_get_number(index)->value);
						StackRow *value = new_stack(da, array->type);
						list_insert(list_end(&stack), value);
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
			
			case BC_PUSH: {
				StackRow *stack_obj = new_stack(new_number(left), DATA_NUMBER);
				list_insert(list_end(&stack), stack_obj);
				debug_log("push %i\n", left);
			}
			break;
			case BC_PUSHSTR: {
				char *string = get_from_constant_list(left);
				StackRow *stack_obj = new_stack(new_string(string), DATA_STRING);
				list_insert(list_end(&stack), stack_obj);
				debug_log("pushstr %s\n", string);
			}
			break;
			case BC_STORE: {
				StackRow *pop = (StackRow*)list_remove(list_back(&stack));
				char *name = get_from_constant_list(left);

				VarList *var = varobject_from_stackobject(pop, name);
				put_var(&varlist, var);
				debug_log("store %s\n", name);
				
				free_stack(pop);
			}
			break;
			case BC_LOAD: {
				char *name = get_from_constant_list(left);
				if(!name) {
					runtime_error("Load failed\n");
				}
				VarList *var = get_var(&varlist, name);
				if(!var) {
					runtime_error("Load var failed\n");
				}

				StackRow *row = stackobject_from_varobject(var);
				list_insert(list_end(&stack), row);

				debug_log("load %s\n", var->name);
			}
			break;
			case BC_STRCONCAT: {
				// mem issue
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				if(pop->type == DATA_STRING && back->type == DATA_STRING) {
					debug_log("strconcat\n");
					concat_string(back->data, stack_get_string(pop)->data, stack_get_string(back)->data);
					free_stack(pop);
				} else {
					runtime_error("concat requires 2 string type");
				}
			}
			break;
			case BC_ADD: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				stack_get_number(back)->value = stack_get_number(pop)->value + stack_get_number(back)->value;

				free_stack(pop);
				debug_log("add\n");
			}
			break;
			case BC_SUB: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				stack_get_number(back)->value = stack_get_number(pop)->value - stack_get_number(back)->value;

				free_stack(pop);
				debug_log("sub\n");
			}
			break;
			case BC_MUL: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				stack_get_number(back)->value = stack_get_number(pop)->value * stack_get_number(back)->value;

				free_stack(pop);
				debug_log("mul\n");
			}
			break;
			case BC_DIV: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				stack_get_number(back)->value = stack_get_number(pop)->value / stack_get_number(back)->value;

				free_stack(pop);
				debug_log("div\n");
			}
			break;
			case BC_CMPEQ: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *pop2  = (StackRow*)list_remove(list_back(&stack));
				if(pop->type == DATA_NUMBER && pop2->type == DATA_NUMBER) {
					if(stack_get_number(pop)->value == stack_get_number(pop2)->value) {
						StackRow *stack_obj = new_stack(new_number(1), DATA_NUMBER);
						list_insert(list_end(&stack), stack_obj);
					} else {
						StackRow *stack_obj = new_stack(new_number(0), DATA_NUMBER);
						list_insert(list_end(&stack), stack_obj);
					}
				} else {
					runtime_error("comparison in cmpgt requires 2 number data type");
				}
				free_stack(pop);
				free_stack(pop2);
				debug_log("cmpgt\n");
			}
			break;
			case BC_CMPNEQ: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *pop2  = (StackRow*)list_remove(list_back(&stack));
				if(pop->type == DATA_NUMBER && pop2->type == DATA_NUMBER) {
					if(stack_get_number(pop)->value != stack_get_number(pop2)->value) {
						StackRow *stack_obj = new_stack(new_number(1), DATA_NUMBER);
						list_insert(list_end(&stack), stack_obj);
					} else {
						StackRow *stack_obj = new_stack(new_number(0), DATA_NUMBER);
						list_insert(list_end(&stack), stack_obj);
					}
				} else {
					runtime_error("comparison in cmpgt requires 2 number data type");
				}
				free_stack(pop);
				free_stack(pop2);
				debug_log("cmpgt\n");
			}
			break;
			case BC_CMPGT: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *pop2  = (StackRow*)list_remove(list_back(&stack));
				if(pop->type == DATA_NUMBER && pop2->type == DATA_NUMBER) {
					if(stack_get_number(pop)->value > stack_get_number(pop2)->value) {
						StackRow *stack_obj = new_stack(new_number(1), DATA_NUMBER);
						list_insert(list_end(&stack), stack_obj);
					} else {
						StackRow *stack_obj = new_stack(new_number(0), DATA_NUMBER);
						list_insert(list_end(&stack), stack_obj);
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
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *pop2  = (StackRow*)list_remove(list_back(&stack));
				if(pop->type == DATA_NUMBER && pop2->type == DATA_NUMBER) {
					if(stack_get_number(pop)->value < stack_get_number(pop2)->value) {
						StackRow *stack_obj = new_stack(new_number(1), DATA_NUMBER);
						list_insert(list_end(&stack), stack_obj);
					} else {
						StackRow *stack_obj = new_stack(new_number(0), DATA_NUMBER);
						list_insert(list_end(&stack), stack_obj);
					}
				} else {
					runtime_error("comparison in cmpgt requires 2 number data type");
				}
				free_stack(pop);
				free_stack(pop2);
				debug_log("cmplt\n");
			}
			break;
			case BC_JMPIFEQ: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *pop2  = (StackRow*)list_remove(list_back(&stack));
				if(pop->type == DATA_NUMBER && pop2->type == DATA_NUMBER) {
					if(stack_get_number(pop)->value == stack_get_number(pop2)->value) {
						i = (ListNode*)get_op_by_index(&function->code, left-1);
					}
				} else {
					runtime_error("comparison in cmpgt requires 2 number data type");
				}
				free_stack(pop);
				free_stack(pop2);
				debug_log("jmpifeq\n");
			}
			break;
			case BC_RET: {
				StackRow *pop = (StackRow*)list_remove(list_back(&stack));
				list_insert(list_end(&transfer), pop);
				debug_log("ret\n");
				goto release;
			}
			break;
			case BC_GOTO: {
				i = (ListNode*)get_op_by_index(&function->code, left-1);
				debug_log("goto\n");
			}
			break;
			case BC_CALL: {
				char *name = get_from_constant_list(left);
				debug_log("call %s\n", name);
				if(name) {
					StackRow *count = (StackRow*)list_remove(list_back(&stack));
					if(strcmp(name, "printf") == 0) {
						StackRow *pop = (StackRow*)list_remove(list_back(&stack));
						if(pop->type == DATA_NUMBER) {
							printf("%i\n", stack_get_number(pop)->value);
						} else if(pop->type == DATA_STRING) {
							printf("%s\n", stack_get_string(pop)->data);
						} else {
							runtime_error("unable to print unsupported type\n");
						}
						free_stack(pop);
					} else if(strcmp(name, "dbgstack") == 0) {
						printf("-----DBGSTACK-----\n");
						for(ListNode *f = list_begin(&stack); f != list_end(&stack); f = list_next(f)) {
							StackRow *row = (StackRow*)f;
							printf("----------\n");
							printf("data_int %i\n", stack_get_number(row)->value);
							printf("----------\n");
						}
					} else if(strcmp(name, "dbgvars") == 0) {
						for(ListNode *g = list_begin(&varlist); g != list_end(&varlist); g = list_next(g)) {
							VarList *row = (VarList*)g;
							printf("----------\n");
							printf("%s data_int %i\n", row->name, var_get_number(row)->value);
							printf("----------\n");
						}
					} else if(strcmp(name, "__callinternal__strlen") == 0) {
						StackRow *str = (StackRow*)list_remove(list_back(&stack));
						if(str->type == DATA_STRING) {
							int str_len = strlen(stack_get_string(str)->data);
							
							StackRow *stack_obj = new_stack(new_number(str_len), DATA_NUMBER);
							list_insert(list_end(&stack), stack_obj);
						} else {
							runtime_error("invalid type passed to __callinternal__strlen");
						}
						free_stack(str);
					} else if(strcmp(name, "__callinternal__charat") == 0) {
						StackRow *index = (StackRow*)list_remove(list_back(&stack));
						StackRow *str = (StackRow*)list_remove(list_back(&stack));
						if(str->type == DATA_STRING && index->type == DATA_NUMBER) {
							char strat = *(stack_get_string(str)->data + stack_get_number(index)->value);
							StackRow *stack_obj = new_stack(new_number((int)strat), DATA_NUMBER);
							list_insert(list_end(&stack), stack_obj);
						} else {
							runtime_error("invalid type passed to __callinternal__charat");
						}
					} else if(strcmp(name, "__callinternal__new_socket") == 0) {
						int fd = socket(AF_INET, SOCK_STREAM, 0);
						if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {

						}
						StackRow *stack_obj = new_stack(new_number(fd), DATA_NUMBER);
						list_insert(list_end(&stack), stack_obj);
					} else if(strcmp(name, "__callinternal__socket_bind") == 0) {
						StackRow *port = (StackRow*)list_remove(list_back(&stack));
						StackRow *ip = (StackRow*)list_remove(list_back(&stack));
						StackRow *fd = (StackRow*)list_remove(list_back(&stack));
						if(fd->type == DATA_NUMBER && ip->type == DATA_STRING && port->type == DATA_NUMBER) {
							struct sockaddr_in addr;
							addr.sin_family = AF_INET; 
							addr.sin_addr.s_addr = inet_addr(stack_get_string(ip)->data); 
							addr.sin_port = htons(stack_get_number(port)->value); 
							if((bind(stack_get_number(fd)->value, (struct sockaddr*)&addr, sizeof(addr))) == 0 && (listen(stack_get_number(fd)->value, 5)) == 0) {
								StackRow *stack_obj = new_stack(new_number(1), DATA_NUMBER);
								list_insert(list_end(&stack), stack_obj);
							} else {
								StackRow *stack_obj = new_stack(new_number(0), DATA_NUMBER);
								list_insert(list_end(&stack), stack_obj);
							}
						} else {
							runtime_error("invalid type passed to __callinternal__socket_bind");
						}
						free_stack(port);
						free_stack(ip);
						free_stack(fd);
					} else if(strcmp(name, "__callinternal__socket_accept") == 0) {
						StackRow *fd = (StackRow*)list_remove(list_back(&stack));
						if(fd->type == DATA_NUMBER) {
							struct sockaddr_in addr;
							int addr_len = sizeof(addr);
							int client = accept(stack_get_number(fd)->value, (struct sockaddr*)&addr, &addr_len); 
							StackRow *stack_obj = new_stack(new_number(client), DATA_NUMBER);
							list_insert(list_end(&stack), stack_obj);
						} else {
							runtime_error("invalid type passed to __callinternal__socket_bind");
						}
						free_stack(fd);
					} else if(strcmp(name, "__callinternal__socket_read") == 0) {
						StackRow *size = (StackRow*)list_remove(list_back(&stack));
						StackRow *fd = (StackRow*)list_remove(list_back(&stack));
						if(fd->type == DATA_NUMBER && size->type == DATA_NUMBER) {
							char buf[stack_get_number(size)->value + 1];
							int s = read(stack_get_number(fd)->value, buf, sizeof(buf));
							buf[sizeof(buf)] = '\0';

							StackRow *stack_obj = new_stack(new_string(buf), DATA_STRING);
							list_insert(list_end(&stack), stack_obj);
						} else {
							runtime_error("invalid type passed to __callinternal__socket_bind");
						}
						free_stack(size);
						free_stack(fd);
					} else if(strcmp(name, "__callinternal__socket_write") == 0) {
						StackRow *data = (StackRow*)list_remove(list_back(&stack));
						StackRow *fd = (StackRow*)list_remove(list_back(&stack));
						if(fd->type == DATA_NUMBER && data->type == DATA_STRING) {
							int s = write(stack_get_number(fd)->value, stack_get_string(data)->data, strlen(stack_get_string(data)->data));
							
							StackRow *stack_obj = new_stack(new_number(s), DATA_NUMBER);
							list_insert(list_end(&stack), stack_obj);
						} else {
							runtime_error("invalid type passed to __callinternal__socket_bind");
						}
						free_stack(data);
						free_stack(fd);
					} else if(strcmp(name, "__callinternal__socket_close") == 0) {
						StackRow *fd = (StackRow*)list_remove(list_back(&stack));
						if(fd->type == DATA_NUMBER) {
							close(stack_get_number(fd)->value);
						} else {
							runtime_error("invalid type passed to __callinternal__socket_bind");
						}
						free_stack(fd);
					} else if(strcmp(name, "__callinternal__itos") == 0) {
						StackRow *num = (StackRow*)list_remove(list_back(&stack));
						if(num->type == DATA_NUMBER) {
							char buf[33];
							sprintf(buf, "%i", stack_get_number(num)->value);
							StackRow *stack_obj = new_stack(new_string(buf), DATA_STRING);
							list_insert(list_end(&stack), stack_obj);
						} else {
							runtime_error("invalid type passed to __callinternal__itos");
						}
						free_stack(num);
					} else if(strcmp(name, "__callinternal__rand") == 0) {
						StackRow *min = (StackRow*)list_remove(list_back(&stack));
						StackRow *max = (StackRow*)list_remove(list_back(&stack));
						if(min->type == DATA_NUMBER && max->type == DATA_NUMBER) {
							int randnum = stack_get_number(min)->value + rand() / (RAND_MAX / (stack_get_number(max)->value - stack_get_number(min)->value + 1) + 1);
							StackRow *stack_obj = new_stack(new_number(randnum), DATA_NUMBER);
							list_insert(list_end(&stack), stack_obj);
						} else {
							runtime_error("invalid type passed to __callinternal__itos");
						}
						free_stack(min);
						free_stack(max);
					} else if(strcmp(name, "__callinternal__exec") == 0) {
						StackRow *cmd = (StackRow*)list_remove(list_back(&stack));
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

							StackRow *stack_obj = new_stack(new_string(res), DATA_STRING);
							list_insert(list_end(&stack), stack_obj);

							free(res);
						} else {
							runtime_error("invalid type passed to __callinternal__itos");
						}
						free_stack(cmd);
					} else {
						if(count && count->type != DATA_NUMBER) {
							runtime_error("Opps, unable to figure number of args to pass to stack\n");
						}
						for(int s = 0; s < stack_get_number(count)->value; s++) {
							StackRow *pop = (StackRow*)list_remove(list_back(&stack));
							list_insert(list_end(&transfer), pop);
						}

						run_binary(left);

						if(list_size(&transfer) > 0) {
							StackRow *ret = (StackRow*)list_remove(list_back(&transfer));
							list_insert(list_end(&stack), ret);
						}
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
		i = list_next(i);
	}

	release:;

	ListNode *va = list_begin(&varlist);
	while(va != list_end(&varlist)) {
		VarList *var = (VarList*)va;
		va = list_next(va);
		list_remove(&var->node);
		free_var(var);
	}
	ListNode *st = list_begin(&stack);
	while(st != list_end(&stack)) {
		StackRow *sta = (StackRow*)st;
		st = list_next(st);
		list_remove(&sta->node);
		free_stack(sta);
	}

	
}

