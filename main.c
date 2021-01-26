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
	FILE *fp = fopen("/home/ryan/compiler/data/out.bin", "rb");
	if(fp == NULL) {
		debug_log("Unable to load\n");
		exit(1);
	}

	list_clear(&constants);
	list_clear(&functions);
	list_clear(&transfer);

	char sig[8];
	fread(sig, sizeof(char), 8, fp);

	unsigned f_count = 0;
	fread(&f_count, sizeof(unsigned), 1, fp);
	for(int i = 0; i < f_count; i++) {
		unsigned f_index = 0;
		unsigned code_count = 0;

		fread(&f_index, sizeof(unsigned), 1, fp);
		fread(&code_count, sizeof(unsigned), 1, fp);

		debug_log("Code Count: %i\n", code_count);
		debug_log("---------------\n");

		Function *function = malloc(sizeof(Function));
		list_clear(&function->code);
		function->index = f_index;

		for(unsigned i = 0; i < code_count; i++) {
			ByteCode op;
			int left;
			int right;
			fread(&op, sizeof(uint8_t), 1, fp);
			fread(&left, sizeof(int), 1, fp);
			fread(&right, sizeof(int), 1, fp);
			

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
	unsigned constant_count = 0;
	fread(&constant_count, sizeof(unsigned), 1, fp);
	debug_log("Constant Count: %i\n", constant_count);
	debug_log("---------------\n");
	for(unsigned i = 0; i < constant_count; i++) {
		unsigned str_len = 0;
		fread(&str_len, sizeof(unsigned), 1, fp);
		char *data = malloc((sizeof(char) * str_len) + 1);
		fread(data, sizeof(char), str_len, fp);

		ConstantPoolRow *row = malloc(sizeof(ConstantPoolRow));
		row->index = i;
		row->data = data;
		list_insert(list_end(&constants), row);
		debug_log("Index: %i, Data: %s\n", i, data);
	}
	
	fclose(fp);
}

void debug_log(const char *format, ...) {
	va_list args;
	va_start(args, format);

	if(1 == 2) {
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
		OP *row = (OP*)i;
		ByteCode op = row->op;
		int left = row->left;
		int right = row->right;
		switch(op) {
			case BC_PUSH: {
				StackRow *stack_obj = new_number_stack_object(left);
				list_insert(list_end(&stack), stack_obj);
				debug_log("push %i\n", left);
			}
			break;
			case BC_PUSHSTR: {
				char *string = get_from_constant_list(left);
				StackRow *stack_obj = new_string_stack_object(string);
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
				char *concat = malloc(sizeof(char) * (strlen(pop->data_string) + strlen(pop->data_string) + 1));
				strcpy(concat, pop->data_string);
				strcat(concat, back->data_string);
				free(back->data_string);
				back->data_string = concat;
				debug_log("strconcat\n");
				free_stack(pop);
			}
			break;
			case BC_ADD: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				back->data_number += pop->data_number;

				free_stack(pop);
				debug_log("add\n");
			}
			break;
			case BC_SUB: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				back->data_number -= pop->data_number;

				free_stack(pop);
				debug_log("sub\n");
			}
			break;
			case BC_MUL: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				back->data_number *= pop->data_number;

				free_stack(pop);
				debug_log("mul\n");
			}
			break;
			case BC_DIV: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				back->data_number /= pop->data_number;

				free_stack(pop);
				debug_log("div\n");
			}
			break;
			case BC_CMPGT: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *pop2  = (StackRow*)list_remove(list_back(&stack));
				if(pop->type == DATA_NUMBER && pop2->type == DATA_NUMBER) {
					if(pop->data_number > pop2->data_number) {
						StackRow *result = new_number_stack_object(1);
						list_insert(list_end(&stack), result);
					} else {
						StackRow *result = new_number_stack_object(0);
						list_insert(list_end(&stack), result);
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
					if(pop->data_number < pop2->data_number) {
						StackRow *result = new_number_stack_object(1);
						list_insert(list_end(&stack), result);
					} else {
						StackRow *result = new_number_stack_object(0);
						list_insert(list_end(&stack), result);
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
					if(pop->data_number == pop2->data_number) {
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
						StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
						if(pop->type == DATA_STRING) {
							printf("%s\n", pop->data_string);
						} else {
							printf("%i\n", pop->data_number);
						}
						free_stack(pop);
					} else if(strcmp(name, "dbgstack") == 0) {
						for(ListNode *f = list_begin(&stack); f != list_end(&stack); f = list_next(f)) {
							StackRow *row = (StackRow*)i;
							printf("----------\n");
							printf("data_int %i\n", row->data_number);
							printf("----------\n");
						}
					} else if(strcmp(name, "dbgvars") == 0) {
						for(ListNode *g = list_begin(&varlist); g != list_end(&varlist); g = list_next(g)) {
							VarList *row = (VarList*)g;
							printf("----------\n");
							printf("%s data_int %i\n", row->name, row->data_number);
							printf("----------\n");
						}
					} else if(strcmp(name, "__callinternal__strlen") == 0) {
						StackRow *str = (StackRow*)list_remove(list_back(&stack));
						if(str->type == DATA_STRING) {
							int str_len = strlen(str->data_string);
							StackRow *res = new_number_stack_object(str_len);
							list_insert(list_end(&transfer), res);
							goto release;
						} else {
							runtime_error("invalid type passed to __callinternal__strlen");
						}
					} else if(strcmp(name, "__callinternal__charat") == 0) {
						StackRow *index = (StackRow*)list_remove(list_back(&stack));
						StackRow *str = (StackRow*)list_remove(list_back(&stack));
						if(str->type == DATA_STRING && index->type == DATA_NUMBER) {
							char strat[2];
							strat[1] = '\0';
							strat[0] = *(str->data_string + index->data_number);
							StackRow *res = new_string_stack_object(strat);
							list_insert(list_end(&transfer), res);
							goto release;
						} else {
							runtime_error("invalid type passed to __callinternal__charat");
						}
					} else {
						if(count && count->type != DATA_NUMBER) {
							runtime_error("Opps, unable to figure number of args to pass to stack\n");
						}
						for(int s = 0; s < count->data_number; s++) {
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

