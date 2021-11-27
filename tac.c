#include <stdlib.h>
#include <string.h>
#include "tac.h"
#include "global.h"
#include "C.tab.h"
#include <stdio.h>
#include "stack.h"

int availableAddresses = 16;
int label_count = 1;

TAC* new_tac(int op, TOKEN* src1, TOKEN* src2, TOKEN* dst){
	TAC* ans = (TAC*)malloc(sizeof(TAC));
	if (ans==NULL) {
		printf("Error! memory not allocated.");
		exit(0);
	}
	ans->op = op;
	TAC_TOKENS tac_tokens = {
		src1,
		src2,
		dst
	};
	ans->args.tokens = tac_tokens;
	char address[4] = "$t";
	char address_num[8];
  	switch (op) {
		case tac_load:
			//ans->args.tokens.dst = dst;
			dst->lexeme = (char*)malloc(4*sizeof(char));

			if (availableAddresses > 0){
				sprintf(address_num, "%d", MAX_ADDRESSES-availableAddresses);
				strcpy(dst->lexeme, address);
				strncat(dst->lexeme, address_num, 1);
				availableAddresses--;
			}
			return ans;

		case tac_load_word:
			//ans->args.tokens.src1 = src1;
			//ans->args.tokens.dst = dst;
			dst->lexeme = (char*)malloc(4*sizeof(char));

			if (availableAddresses > 0){
				sprintf(address_num, "%d", MAX_ADDRESSES-availableAddresses);
				strcpy(dst->lexeme, address);
				strncat(dst->lexeme, address_num, 1);
				availableAddresses--;
			}
			return ans;

		default:
			return ans;
  	}
}

TOKEN* generate_label(){
	TOKEN* token = new_token(3);
	token->lexeme = malloc(sizeof(char)*5);
	sprintf(token->lexeme, "L%d", label_count++);
	return token;
}

TAC* new_proc_tac(int op, TOKEN* name, int arity){
	TAC* ans = (TAC*)malloc(sizeof(TAC));
	ans->op = op;
	CALL call = {
		name,
		arity
	};
	ans->args.call = call;
	return ans;
}

void attach_tac(TAC* left, TAC* right){
	if (left->next == NULL) {
		left->next = right;
	} else {
		attach_tac(left->next, right);
	}
}

TAC* end_tac(TAC* start){
	if (start->next == NULL){
		return start;
	} else {
		return end_tac(start->next);
	}
}

TAC* arithmetic_tac(NODE* ast, int op){
	TAC* left_operand = mmc_icg(ast->left);
	TAC* right_operand = mmc_icg(ast->right);
	TAC* operator = new_tac(op, left_operand->args.tokens.dst, right_operand->args.tokens.dst, left_operand->args.tokens.dst);
	attach_tac(left_operand, right_operand);
	attach_tac(right_operand, operator);
	return left_operand;
}

int count_args(NODE* args){
	int count = 0;
	if (args == NULL || args->left->type == VOID){
		return 0;
	} else if (args->type == ASSIGNMENT){
		return 1;
	} else if (args->type == 44){
		count += count_args(args->left);
		count += count_args(args->right);
		return count;
	}
}

int count_vars(NODE* args){
	printf("Start: %p, %d\n", args, args->type);
	int count = 0;
	if (args == NULL){
		printf("Empty node\n");
		return 0;
	} else if (args->type == IDENTIFIER){
		printf("ID found\n");
		return 1;
	} else {
		printf("Search instead, left: %p, right: %p\n", args->left, args->right);
		count += count_vars(args->left);
		count += count_vars(args->right);
		return count;
	}
}

TAC* mmc_icg(NODE* ast) // NOTE: With jumps, we need to determine where we need to jump to.
{
  	switch (ast->type) {
		case 68: //D
			printf("Begin TAC Construction.\n");
			TAC* proc_def = mmc_icg(ast->left);
			TAC* proc_body = mmc_icg(ast->right);
			// int nvars = count_vars(ast->right);
			// printf("Number of variables: %d\n", nvars);

			proc_def->next = proc_body;
			TAC* end_proc = new_tac(tac_proc_end, NULL, NULL, NULL);
			attach_tac(proc_body, end_proc);
			return proc_def;
		case 100: //d
			printf("Function definition.\n");
			return mmc_icg(ast->right);
		
		case 70:; //F
			TAC* proc = new_proc_tac(tac_proc, (TOKEN*)ast->left->left, count_args(ast->right));
			return proc;
		case VOID:;
			TOKEN* void_token = (TOKEN*)ast;
			TAC* void_tac = new_tac(tac_arg, NULL, NULL, void_token);
			return void_tac;
		case RETURN:
			printf("Return found.\n");
			TAC* tac_process_to_return = mmc_icg(ast->left);
			
			TAC* ret = new_tac(tac_return, NULL, NULL, tac_process_to_return->args.tokens.dst);
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
			TAC* assign = new_tac(tac_store_word, load->args.tokens.dst, NULL, variable->args.tokens.src1);
			variable->next = load;
			attach_tac(load, assign);
			return variable;

		case IDENTIFIER:
			printf("Identifier found.\n");

			TOKEN* reg_id = (TOKEN*)malloc(sizeof(TOKEN));
			return new_tac(tac_load_word, (TOKEN*)ast, NULL, reg_id);
		case IF:
			printf("If found.\n");
			TAC* condition = mmc_icg(ast->left);
			TAC* condition_type = end_tac(condition); // Get the boolean equality operator
			TOKEN* label = generate_label();
			TAC* alt_jump = new_tac(tac_goto, NULL, NULL, label); // This is the jump where the condition isn't met

			
			TAC* body; // Get the body of the if statement
			TAC* alt_body; // Possible else body
			if (ast->right->type != ELSE){
				body = mmc_icg(ast->right);
				attach_tac(condition, alt_jump);
			} else {
				body = mmc_icg(ast->right->left);
				alt_body = mmc_icg(ast->right->right);
				attach_tac(condition, alt_body);
				attach_tac(alt_body, alt_jump);
			}
			
			// Generate the label if condition is true
			TAC* true_label = new_tac(tac_label, NULL, NULL, condition_type->args.tokens.dst);
			alt_jump->next = true_label;
			true_label->next = body;

			// Generate the label if the label is false
			TAC* false_label = new_tac(tac_label, NULL, NULL, label);
			attach_tac(body, false_label);

			return condition;
		
		case EQ_OP:;
			TAC* left_op = mmc_icg(ast->left);
			TAC* right_op = mmc_icg(ast->right);
			
			TAC* equality = new_tac(tac_equality, left_op->args.tokens.dst, right_op->args.tokens.dst, generate_label());
			attach_tac(left_op, right_op);
			right_op->next = equality;
			return left_op;
		case ELSE:;

		default:
			printf("unknown type code %d (%p) in mmc_icg\n",ast->type,ast);
			return NULL;
  	}
}

BB* new_basic_block(TAC* tac){
	BB* bb = (BB*)malloc(sizeof(BB));
	bb->leader = tac;
	bb->next = NULL;
	return bb;
}

BB* block_graph_gen(TAC* tac){
	BB* bb = new_basic_block(tac);
	BB* next = bb->next;
	tac = tac->next;

	for(TAC* i = tac; i != NULL; i = i->next){
		if (i->op == tac_proc){
			next = new_basic_block(i);
			next = next->next;
		}
	}
	return bb;
}

void print_blocks(BB* bb){
	TAC* tac = bb->leader;
	int i = 0;
	printf("Basic Block %d\n", i);
	while(bb != NULL){

		if (tac != NULL && (bb->next == NULL || tac->next != bb->next->leader)){
			print_single_tac(tac);
			tac = tac->next;
		} else {
			
			bb = bb->next;
			i++;
			if (bb != NULL){
				printf("\nBasic Block %d\n", i);
			}
		}
	}
}

//TODO: Optimise with seperate passes.
void optimise_block(BB* bb){
	TAC* leader = bb->leader;
	STACK* stack = new_stack();
	STACK* add_stack = new_stack();
	for(TAC* i = leader; i != NULL; i=i->next){
		if (i->op == tac_plus){
			TAC* add = i;
			TAC* load2 = (TAC*)pop(stack);
			TAC* load1 = (TAC*)pop(stack);
			// If the values of the arguments in the loads before the add are the same, then we only need one load.
			if (load1->op == tac_load && load2->op == tac_load && load1->args.tokens.dst->value == load2->args.tokens.dst->value){
				add->args.tokens.src2 = add->args.tokens.src1;
				free(load2);
				load1->next = add;
				push(stack, (void*)load1);
				push(stack, (void*)i);
			} else {
				push(stack, (void*)load1);
				push(stack, (void*)load2);
				push(stack, (void*)i);
			}
			// push(add_stack, i);
			// if (add_stack->top > 0){
			// 	TAC* add2 = (TAC*)pop(add_stack);
			// 	TAC* add1 = (TAC*)pop(add_stack);
			// 	if (add2->args.tokens.src1->value == add1->args.tokens.src1->value && add2->args.tokens.src2->value == add1->args.tokens.src2->value){
			// 		TAC* move_tac = new_tac(tac_move, add1->args.tokens.dst, NULL, add2->args.tokens.dst);
			// 		TAC* prev_tac = (TAC*)pop(stack);
			// 		move_tac->next = prev_tac->next;
			// 		prev_tac->next = move_tac;
			// 	}
			// }
		} else if (i->op != tac_proc && i->op != tac_proc_end){
			push(stack, (void*)i);
		}
	}
	printf("\nOptimised:\n");
	mmc_print_ic(leader);
}

void print_single_tac(TAC* i){
	if (i->op == tac_plus || i->op == tac_minus || i->op == tac_divide || i->op == tac_multiply || i->op == tac_mod){
		printf("%s %s, %s, %s\n",
		tac_ops[i->op], // need to range check!
		i->args.tokens.src1->lexeme,
		i->args.tokens.src2->lexeme,
		i->args.tokens.dst->lexeme);
	} else if (i->op == tac_load){
		printf("%s %s, %d\n",
		tac_ops[i->op],
		i->args.tokens.dst->lexeme,
		i->args.tokens.dst->value);
	} else if (i->op == tac_return || i->op == tac_label || i->op == tac_goto){
		printf("%s %s\n",
		tac_ops[i->op],
		i->args.tokens.dst->lexeme);
	}else if (i->op == tac_store_word || i->op == tac_move){
		if (i->args.tokens.src1 != NULL){
			printf("%s %s, %s\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.dst->lexeme);
		} else {
			printf("%s %d, %s\n", tac_ops[i->op], 0, i->args.tokens.dst->lexeme);
		}
	} else if (i->op == tac_proc){
		printf("%s %s %d\n", tac_ops[i->op], i->args.call.name->lexeme, i->args.call.arity);
	} else if (i->op == tac_load_word){
		printf("%s %s, %s\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.dst->lexeme);
	} else if (i->op == tac_proc_end){
		printf("%s\n", tac_ops[i->op]);
	} else if(i->op == tac_equality){
		printf("%s %s == %s %s\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.src2->lexeme, i->args.tokens.dst->lexeme);
	}
}

void mmc_print_ic(TAC* i)
{
  	for(;i!=NULL;i=i->next)
	print_single_tac(i);
}