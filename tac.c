#include <stdlib.h>
#include <string.h>
#include "tac.h"
#include "global.h"
#include "C.tab.h"
#include <stdio.h>

int availableAddresses = 16;

TAC* new_tac(int op, TOKEN* src1, TOKEN* src2, TOKEN* dst){
	TAC* ans = (TAC*)malloc(sizeof(TAC));
	if (ans==NULL) {
		printf("Error! memory not allocated.");
		exit(0);
	}
	ans->op = op;
	char address[4] = "$t";
	char address_num[8];
  	switch (op) {
		case tac_load:
			ans->dst = dst;
			dst->lexeme = (char*)malloc(4*sizeof(char));

			if (availableAddresses > 0){
				sprintf(address_num, "%d", MAX_ADDRESSES-availableAddresses);
				strcpy(dst->lexeme, address);
				strncat(dst->lexeme, address_num, 1);
				availableAddresses--;
			}
			return ans;

		case tac_load_word:
			ans->src1 = src1;
			ans->dst = dst;
			dst->lexeme = (char*)malloc(4*sizeof(char));

			if (availableAddresses > 0){
				sprintf(address_num, "%d", MAX_ADDRESSES-availableAddresses);
				strcpy(dst->lexeme, address);
				strncat(dst->lexeme, address_num, 1);
				availableAddresses--;
			}
			return ans;

		default:
			ans->dst = dst;
			ans->src1 = src1;
			ans->src2 = src2;
			return ans;
  	}
}

void attach_tac(TAC* left, TAC* right){
	if (left->next == NULL) {
		left->next = right;
	} else {
		attach_tac(left->next, right);
	}
}

TAC* arithmetic_tac(NODE* ast, int op){
	TAC* left_operand = mmc_icg(ast->left);
	TAC* right_operand = mmc_icg(ast->right);
	TAC* operator = new_tac(op, left_operand->dst, right_operand->dst, left_operand->dst);
	attach_tac(left_operand, right_operand);
	attach_tac(right_operand, operator);
	return left_operand;
}

TAC* mmc_icg(NODE* ast) // NOTE: With jumps, we need to determine where we need to jump to.
{
  	switch (ast->type) {
		case 68: //D
			printf("Begin TAC Construction.\n");
			TAC* proc_def = mmc_icg(ast->left);
			TAC* proc_body = mmc_icg(ast->right);
			proc_def->next = proc_body;
			TAC* end_proc = new_tac(tac_proc_end, NULL, NULL, NULL);
			attach_tac(proc_body, end_proc);
			return proc_def;
		case 100: //d
			printf("Function definition.\n");
			return mmc_icg(ast->right);
		
		case 70:; //F
			TAC* proc = new_tac(tac_proc, NULL, NULL, mmc_icg(ast->right)->dst);
			TOKEN* proc_name = (TOKEN*)malloc(sizeof(TOKEN));
			//printf("%d\n", ast->left->left->type);
			proc_name->lexeme = ((TOKEN*)ast->left->left)->lexeme;
			proc->src1 = proc_name;
			return proc;
		case VOID:;
			TOKEN* void_token = (TOKEN*)ast;
			TAC* void_tac = new_tac(tac_arg, NULL, NULL, void_token);
			return void_tac;
		case RETURN:
			printf("Return found.\n");
			TAC* tac_process_to_return = mmc_icg(ast->left);
			
			TAC* ret = new_tac(tac_return, NULL, NULL, tac_process_to_return->dst);
			attach_tac(tac_process_to_return, ret);
			return tac_process_to_return;

		case LEAF:
			printf("Leaf found.\n");
			return mmc_icg(ast->left);

		case 43: //+
			printf("Plus found.\n");
			return arithmetic_tac(ast, tac_plus);

		case CONSTANT:
			printf("Constant found.\n");
			return new_tac(tac_load, NULL, NULL, (TOKEN *) ast);
		case 45: //-
		 	printf("Minus found.\n");
			return arithmetic_tac(ast, tac_minus);
		  
		case 47: //(/)
			printf("Divide found.\n");
			return arithmetic_tac(ast, tac_divide);
		  
		case 42: //(*)
			printf("Multiplication found.\n");
			return arithmetic_tac(ast, tac_multiply);
		case 37: //%
		  	printf("Modulo found.\n");
			return arithmetic_tac(ast, tac_mod);

		case 59: //;
			printf("Sequence found.\n");
			TAC* left_seq = mmc_icg(ast->left);
			TAC* right_seq = mmc_icg(ast->right); // Right block
			attach_tac(left_seq, right_seq);
			return left_seq;

		case ASSIGNMENT: // ~
			printf("Assignment found.\n");
			if (ast->right->type == 61){ // if equals
				return mmc_icg(ast->right);
			} else {
				TAC* variable_declare = new_tac(tac_declare, NULL, NULL, NULL);
				return variable_declare;
			}

		case 61: // =
			printf("Equals found.\n");
			
			TAC* variable = mmc_icg(ast->left);
			TAC* load = mmc_icg(ast->right);
			TAC* assign = new_tac(tac_store_word, load->dst, NULL, variable->src1);
			variable->next = load;
			attach_tac(load, assign);
			return variable;

		case IDENTIFIER:
			printf("Identifier found.\n");

			TOKEN* reg_id = (TOKEN*)malloc(sizeof(TAC));
			//TAC* load_word_id = new_tac(tac_load_word, ());
			return new_tac(tac_load_word, (TOKEN*)ast, NULL, reg_id);
		default:
			printf("unknown type code %d (%p) in mmc_icg\n",ast->type,ast);
			return NULL;
  	}
}

void mmc_print_ic(TAC* i)
{
  	for(;i!=NULL;i=i->next)
	if (i->op == tac_plus || i->op == tac_minus || i->op == tac_divide || i->op == tac_multiply || i->op == tac_mod){
		printf("%s %s, %s, %s\n",
		tac_ops[i->op], // need to range check!
		i->src1->lexeme,
		i->src2->lexeme,
		i->dst->lexeme);
	} else if (i->op == tac_load){
		printf("%s %s, %d\n",
		tac_ops[i->op],
		i->dst->lexeme,
		i->dst->value);
	} else if (i->op == tac_return){
		printf("%s %s\n",
		tac_ops[i->op],
		i->dst->lexeme);
	}else if (i->op == tac_store_word){
		if (i->src1 != NULL){
			printf("%s %s, %s\n", tac_ops[i->op], i->src1->lexeme, i->dst->lexeme);
		} else {
			printf("%s %d, %s\n", tac_ops[i->op], 0, i->dst->lexeme);
		}
	} else if (i->op == tac_proc || i->op == tac_load_word){
		printf("%s %s, %s\n", tac_ops[i->op], i->src1->lexeme, i->dst->lexeme);
	} else if (i->op == tac_proc_end){
		printf("%s\n", tac_ops[i->op]);
	}
}