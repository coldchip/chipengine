#include "main.h"

List constants;
List functions;
List transfer;

int main(int argc, char const *argv[]) {
	/* code */
	load_binary();
	
	unsigned f_index = get_char_from_constant_list("main");
	run_binary(f_index);
	
	return 0;
}

void runtime_error(const char* data) {
	printf("%s\n", data);
	exit(1);
}

void load_binary() {
	FILE *fp = fopen("/home/ryan/compiler/data/out.bin", "rb");
	if(fp == NULL) {
		printf("Unable to load\n");
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

		printf("Code Count: %i\n", code_count);
		printf("---------------\n");

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
			printf("%i %i %i\n", op, left, right);
		}
		list_insert(list_end(&functions), function);
	}
	printf("\n\n");
	unsigned constant_count = 0;
	fread(&constant_count, sizeof(unsigned), 1, fp);
	printf("Constant Count: %i\n", constant_count);
	printf("---------------\n");
	for(unsigned i = 0; i < constant_count; i++) {
		unsigned str_len = 0;
		fread(&str_len, sizeof(unsigned), 1, fp);
		char *data = malloc(sizeof(char) * str_len);
		fread(data, sizeof(char), str_len, fp);

		ConstantPoolRow *row = malloc(sizeof(ConstantPoolRow));
		row->index = i;
		row->data = data;
		list_insert(list_end(&constants), row);
		printf("Index: %i, Data: %s\n", i, data);
	}
	
	fclose(fp);
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
	printf("index out of range\n");
	exit(1);
}

void run_binary(int index) {
	Function *function = get_function(index);
	if(!function) {
		printf("Function %i not found\n", index);
		exit(1);
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
				printf("push %i\n", left);
			}
			break;
			case BC_PUSHSTR: {
				char *string = get_from_constant_list(left);
				StackRow *stack_obj = new_string_stack_object(string);
				list_insert(list_end(&stack), stack_obj);
				printf("pushstr %s\n", string);
			}
			break;
			case BC_STORE: {
				StackRow *pop = (StackRow*)list_remove(list_back(&stack));
				
				char *var_name = get_from_constant_list(left);

				VarList *var = varobject_from_stackobject(pop, var_name);
				list_insert(list_end(&varlist), var);
				printf("store %s\n", var_name);
				
				free(pop);
			}
			break;
			case BC_LOAD: {
				char *name = get_from_constant_list(left);
				if(!name) {
					printf("Load failed\n");
					exit(1);
				}
				VarList *var = get_var(&varlist, name);
				if(!var) {
					printf("Load var failed\n");
					exit(1);
				}

				StackRow *row = stackobject_from_varobject(var);
				list_insert(list_end(&stack), row);

				printf("load %s\n", var->name);
			}
			break;
			case BC_STRCONCAT: {
				// mem issue
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				char *concat = malloc(sizeof(char) * (strlen(pop->data_string) + strlen(pop->data_string) + 1));
				strcpy(concat, pop->data_string);
				strcat(concat, back->data_string);
				back->data_string = concat;
				printf("strconcat\n");
				free(pop);
			}
			break;
			case BC_ADD: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				back->data_number += pop->data_number;

				free(pop);
				printf("add\n");
			}
			break;
			case BC_SUB: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				back->data_number -= pop->data_number;

				free(pop);
				printf("sub\n");
			}
			break;
			case BC_MUL: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				back->data_number *= pop->data_number;

				free(pop);
				printf("mul\n");
			}
			break;
			case BC_DIV: {
				StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
				StackRow *back = (StackRow*)list_back(&stack);
				back->data_number /= pop->data_number;

				free(pop);
				printf("div\n");
			}
			break;
			case BC_RET: {
				StackRow *pop = (StackRow*)list_remove(list_back(&stack));
				list_insert(list_end(&transfer), pop);
				printf("ret\n");
				return;
			}
			break;
			case BC_GOTO: {
				i = get_op_by_index(&function->code, left-1);
				printf("goto\n");
			}
			break;
			case BC_CALL: {
				char *name = get_from_constant_list(left);
				printf("call\n");
				if(strcmp(name, "printf") == 0) {
					StackRow *count  = (StackRow*)list_remove(list_back(&stack));
					StackRow *pop  = (StackRow*)list_remove(list_back(&stack));
					if(pop->type == DATA_STRING) {
						printf("--print %s\n", pop->data_string);
					} else {
						printf("--print %i\n", pop->data_number);
					}
				} else if(strcmp(name, "dbgstack") == 0) {
					for(ListNode *i = list_begin(&stack); i != list_end(&stack); i = list_next(i)) {
						StackRow *row = (StackRow*)i;
						printf("----------\n");
						printf("data_int %i\n", row->data_number);
						printf("----------\n");
					}
				} else if(strcmp(name, "dbgvars") == 0) {
					for(ListNode *i = list_begin(&varlist); i != list_end(&varlist); i = list_next(i)) {
						VarList *row = (VarList*)i;
						printf("----------\n");
						printf("%s data_int %i\n", row->name, row->data_number);
						printf("----------\n");
					}
				} else {
					StackRow *count = (StackRow*)list_remove(list_back(&stack));
					if(count && count->type != DATA_NUMBER) {
						printf("Opps, unable to figure number of args to pass to stack\n");
					}
					for(int i = 0; i < count->data_number; i++) {
						StackRow *pop = (StackRow*)list_remove(list_back(&stack));
						list_insert(list_end(&transfer), pop);
					}

					run_binary(left);

					if(list_size(&transfer) > 0) {
						StackRow *ret = (StackRow*)list_remove(list_back(&transfer));
						list_insert(list_end(&stack), ret);
					}
				}

			}
			break;
		}
		i = list_next(i);
	}
}

