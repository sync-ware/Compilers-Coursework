#include <stdlib.h>
#include <string.h>
#include "tac.h"
//#include "global.h"
#include "C.tab.h"
#include <stdio.h>

int availableAddresses = 8;

TAC* new_tac(int op, TOKEN* src1, TOKEN* src2, TOKEN* dst){
	TAC* ans = (TAC*)malloc(sizeof(TAC));
	if (ans==NULL) {
		printf("Error! memory not allocated.");
		exit(0);
	}
	ans->op = op;
  	switch (op) {
		case tac_plus:
			ans->src1 = src1;
			ans->src2 = src2;
			ans->dst = dst;
			return ans;

		case tac_minus:
			ans->src1 = src1;
			ans->src2 = src2;
			ans->dst = dst;
			return ans;

		case tac_divide:
			ans->src1 = src1;
			ans->src2 = src2;
			ans->dst = dst;
			return ans;

		case tac_multiply:
			ans->src1 = src1;
			ans->src2 = src2;
			ans->dst = dst;
			return ans;

		case tac_mod:
			ans->src1 = src1;
			ans->src2 = src2;
			ans->dst = dst;
			return ans;

		case tac_load:
			ans->dst = dst;
			dst->lexeme = (char*)malloc(4*sizeof(char));
			char address_num[8];
			char address[4] = "$t";
	
			if (availableAddresses > 0){
				sprintf(address_num, "%d", MAX_ADDRESSES-availableAddresses);
				strcpy(dst->lexeme, address);
				strncat(dst->lexeme, address_num, 1);
				availableAddresses--;
			}
			return ans;

		case tac_return:
			ans->dst = dst;
			return ans;

		case tac_declare:
			ans->dst = dst;
			//ans->src1 = src1;
			return ans;
		case tac_assign:
			ans->dst = dst;
			ans->src1 = src1;
			return ans;
		case tac_variable:
			ans->dst = dst;
			return ans;
		default:
			return NULL;
  	}

}

void attach_tac(TAC* left, TAC* right){
	if (left->next == NULL) {
		left->next = right;
	} else {
		attach_tac(left->next, right);
	}
}

TAC* mmc_icg(NODE* ast) // NOTE: With jumps, we need to determine where we need to jump to.
{
  	switch (ast->type) {
		case 68: //D
			printf("Begin TAC Construction.\n");
			return mmc_icg(ast->right);

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
			TAC* left_plus = mmc_icg(ast->left);

			TAC* right_plus = mmc_icg(ast->right);

			TAC* add = new_tac(tac_plus, left_plus->dst, right_plus->dst, left_plus->dst);

			// We must iterate through to the end of the left tacs.
			attach_tac(left_plus, right_plus);
			right_plus->next = add;
			return left_plus;

		case CONSTANT:
			printf("Constant found.\n");
			return new_tac(tac_load, NULL, NULL, (TOKEN *) ast);
		case 45: //-
		 	printf("Minus found.\n");
		  	TAC* left_sub = mmc_icg(ast->left);

			TAC* right_sub = mmc_icg(ast->right);

			TAC* minus = new_tac(tac_minus, left_sub->dst, right_sub->dst, left_sub->dst);

			// We must iterate through to the end of the left tacs.
			attach_tac(left_sub, right_sub);
			right_sub->next = minus;
			return left_sub;
		  
		case 47: //(/)
			printf("Divide found.\n");
			TAC* left_div = mmc_icg(ast->left);

			TAC* right_div = mmc_icg(ast->right);

			TAC* divide = new_tac(tac_divide, left_div->dst, right_div->dst, left_div->dst);

			// We must iterate through to the end of the left tacs.
			attach_tac(left_div, right_div);
			right_div->next = divide;
			return left_div;
		  
		case 42: //(*)
			printf("Multiplication found.\n");
		  	TAC* left_multi = mmc_icg(ast->left);

			TAC* right_multi = mmc_icg(ast->right);

			TAC* multiply = new_tac(tac_multiply, left_multi->dst, right_multi->dst, left_multi->dst);

			// We must iterate through to the end of the left tacs.
			attach_tac(left_multi, right_multi);
			right_multi->next = multiply;
			return left_multi;
		case 37: //%
		  	printf("Modulo found.\n");
		  	TAC* left_mod = mmc_icg(ast->left);

			TAC* right_mod = mmc_icg(ast->right);

			TAC* mod = new_tac(tac_mod, left_mod->dst, right_mod->dst, left_mod->dst);

			// We must iterate through to the end of the left tacs.
			attach_tac(left_mod, right_mod);
			right_mod->next = mod;
			return left_mod;

		case 59: //;
			printf("Sequence found.\n");
			TAC* left_seq = mmc_icg(ast->left);
			TAC* right_seq = mmc_icg(ast->right); // Right block
			attach_tac(left_seq, right_seq);
			return left_seq;
		case ASSIGNMENT: // ~
			printf("Assignment found.\n");
			return mmc_icg(ast->right);
		case 61: // =
			printf("Equals found.\n");
			
			TAC* variable = mmc_icg(ast->left);;
			TAC* declare = new_tac(tac_declare, NULL, NULL, variable->dst);
			TAC* load = mmc_icg(ast->right);
			TAC* assign = new_tac(tac_assign, declare->dst, NULL, load->dst);
			attach_tac(load, declare);
			declare->next = assign;
			return load;
		case IDENTIFIER:
			printf("Identifier found.\n");

			return new_tac(tac_variable, NULL, NULL, (TOKEN*)ast);
		default:
			printf("unknown type code %d (%p) in mmc_icg\n",ast->type,ast);
			return NULL;
  	}
};