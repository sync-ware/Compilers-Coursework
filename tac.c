#include <stdlib.h>
#include <string.h>
#include "tac.h"
#include "global.h"
#include "C.tab.h"
#include <stdio.h>
#include <unistd.h>

int availableAddresses = 9;
int label_count = 1;

// Create a new TAC instruction based on the arguments given
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
	if (availableAddresses == 0){
		availableAddresses = 9;
	}
  	switch (op) {
		case tac_load:
			dst->lexeme = (char*)malloc(4*sizeof(char));

			if (availableAddresses > 0){
				// Generate a temp register
				sprintf(address_num, "%d", MAX_ADDRESSES-availableAddresses);
				strcpy(dst->lexeme, address);
				strncat(dst->lexeme, address_num, 1);
				availableAddresses--;
			}
			return ans;

		case tac_load_word:
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

// Create a unique label
TOKEN* generate_label(){
	TOKEN* token = new_token(3);
	token->lexeme = malloc(sizeof(char)*5);
	sprintf(token->lexeme, "L%d", label_count++);
	return token;
}

// A new TAC for proc defenitions. Supply a stack with arguments so that we can store it
TAC* new_proc_tac(int op, TOKEN* name, STACK* arg_stack){
	TAC* ans = (TAC*)malloc(sizeof(TAC));
	ans->op = op;
	char** arg_names = malloc(sizeof(char*)*arg_stack->size);
	TAC* curr_tac = ans;
	for(int i = 0; i < arg_stack->size; i++){
		arg_names[i] = (char*)pop(arg_stack);
		TOKEN* var = (TOKEN*)malloc(sizeof(TOKEN));
		var->lexeme = arg_names[i];

		// Store the value into an argument register
		TOKEN* arg = (TOKEN*)malloc(sizeof(TOKEN));
		arg->lexeme = malloc(sizeof(char)*4);
		sprintf(arg->lexeme, "$a%d", i);
		TAC* sw = new_tac(tac_store_word, arg, NULL, var);
		curr_tac->next = sw;
		curr_tac = sw;
	}
	CALL* call = (CALL*)malloc(sizeof(CALL));
	call->arg_names = arg_names;
	call->arity = arg_stack->size;
	call->name = name;
	ans->args.call = call;
	return ans;
}

// End proc TAC, shares properties with the Proc TAC
TAC* new_end_proc_tac(int op, CALL* call)
{
	TAC* ans = (TAC*)malloc(sizeof(TAC));
	ans->op = op;
	ans->args.call = call;
	return ans;
}

// Attack two TAC blocks together
void attach_tac(TAC* left, TAC* right){
	if (left->next == NULL) {
		left->next = right;
	} else {
		attach_tac(left->next, right);
	}
}

// Get the last TAC in a sequence
TAC* end_tac(TAC* start){
	if (start->next == NULL){
		return start;
	} else {
		return end_tac(start->next);
	}
}

// TAC for arithmetic instructions
TAC* arithmetic_tac(NODE* ast, int op){
	TAC* left_operand = mmc_icg(ast->left);
	TAC* right_operand = mmc_icg(ast->right);
	TAC* operator = new_tac(
		op, 
		left_operand->args.tokens.dst, 
		right_operand->args.tokens.dst, 
		left_operand->args.tokens.dst
	);
	attach_tac(left_operand, right_operand);
	attach_tac(right_operand, operator);
	return left_operand;
}

// Process the arguments of a function and place them on a stack
void generate_args(NODE* args, STACK* stack){
	if (args != NULL && args->type == ASSIGNMENT){
		push(stack, (void*)((TOKEN*)args->right->left)->lexeme);
	} else if (args != NULL && args->type == 44){
		generate_args(args->left, stack);
		generate_args(args->right, stack);
	}
}

// Create a TAC for IF statements
TAC* conditonal_tac(NODE* ast, int type){
	// We need to load what we are comparing into temps
	TAC* left_op = mmc_icg(ast->left);
	TAC* right_op = mmc_icg(ast->right);
	// IF TAC
	TAC* equality = new_tac(type, left_op->args.tokens.dst, right_op->args.tokens.dst, generate_label());
	attach_tac(left_op, right_op);
	right_op->next = equality;
	return left_op;
}

// Count the number of arg resgisters used
int args_count = 0;

// TAC generator main
TAC* mmc_icg(NODE* ast)
{
  	switch (ast->type) {
		case 68:; //D
			TAC* proc_def = mmc_icg(ast->left);
			args_count = 0; // Reset arg count for new function
			TAC* proc_body = mmc_icg(ast->right);
			attach_tac(proc_def, proc_body);
			// End PROC requires details so that we can restore the stack in the MC
			TAC* end_proc = new_end_proc_tac(tac_proc_end, proc_def->args.call);
			// Main has a different end tac because MC doesn't do anything special with it
			if (strcmp(proc_def->args.call->name->lexeme, "main") == 0){
				end_proc = new_tac(tac_main_end, NULL, NULL, NULL);
			}
			attach_tac(proc_body, end_proc);
			return proc_def;
		case 100: //d
			return mmc_icg(ast->right);
		
		case 70:; //F
			// Get the function arguments
			STACK* stack = new_stack();
			generate_args(ast->right, stack);
			// Proc defenition
			TAC* proc = new_proc_tac(tac_proc, (TOKEN*)ast->left->left, stack);
			return proc;
		case VOID:;
			// Not used, but indicates that we found a VOID
			TOKEN* void_token = (TOKEN*)ast;
			TAC* void_tac = new_tac(tac_arg, NULL, NULL, void_token);
			return void_tac;
		case RETURN:;
			// Return from function TAC
			TAC* tac_process_to_return = mmc_icg(ast->left);
			// Temp register to return, we will just translate that as a load
			TAC* ret = new_tac(tac_return, NULL, NULL, tac_process_to_return->args.tokens.dst);
			
			attach_tac(tac_process_to_return, ret);
			return tac_process_to_return;

		case LEAF:
			return mmc_icg(ast->left);

		case 43: //+
			return arithmetic_tac(ast, tac_plus);

		case CONSTANT:
			return new_tac(tac_load, NULL, NULL, (TOKEN *) ast);
		case 45: //-
			return arithmetic_tac(ast, tac_minus);
		  
		case 47: //(/)
			return arithmetic_tac(ast, tac_divide);
		  
		case 42: //(*)
			return arithmetic_tac(ast, tac_multiply);
		case 37: //%
			return arithmetic_tac(ast, tac_mod);

		case 59:; //;
			TAC* left_seq = mmc_icg(ast->left); // Left block
			TAC* right_seq = mmc_icg(ast->right); // Right block
			attach_tac(left_seq, right_seq);
			return left_seq;

		case ASSIGNMENT:; // ~
			// Like the interpreter, we must account for function declarations
			if (ast->left->type == ASSIGNMENT  || ast->left->type == 68){
				TAC* left_func = mmc_icg(ast->left);
				TAC* right_func = mmc_icg(ast->right);
				attach_tac(left_func, right_func);
				return left_func;
			} else { // Or an assignment to a variable
				if (ast->right->type == 61){ // if equals
					return mmc_icg(ast->right);
				} else {
					// Declare variable TAC, will be used in conjuction with an (=) in the AST
					TAC* variable_declare = new_tac(tac_declare, NULL, NULL, NULL);
					return variable_declare;
				}
			}

		case 61:; // =
			// Load a value into a temp, then put it into a word
			TAC* variable = mmc_icg(ast->left);
			TAC* load = mmc_icg(ast->right);
			TAC* assign = new_tac(tac_store_word, load->args.tokens.dst, NULL, variable->args.tokens.src1);
			variable->next = load;
			attach_tac(load, assign);
			return variable;

		case IDENTIFIER:;
			// Load a word into a temp reg

			TOKEN* reg_id = (TOKEN*)malloc(sizeof(TOKEN));
			return new_tac(tac_load_word, (TOKEN*)ast, NULL, reg_id);
		case IF:;
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
			return conditonal_tac(ast, tac_equality);
		case ELSE:;
			break;
		case LE_OP:
			return conditonal_tac(ast, tac_le_op);

		case GE_OP:
			return conditonal_tac(ast, tac_ge_op);

		case NE_OP:
			return conditonal_tac(ast, tac_n_equality);

		case 62:
			return conditonal_tac(ast, tac_gt_op);

		case 60:
			return conditonal_tac(ast, tac_lt_op);
		case APPLY:;
			TAC* id = mmc_icg(ast->left);
			TOKEN* ret_v = (TOKEN*)malloc(sizeof(TOKEN));
			ret_v->lexeme = malloc(sizeof(char)*5);
			ret_v->lexeme = "$v0";
			// We need to store the register where the value of the apply should go,
			// this is for other TACs
			TAC* apply = new_tac(tac_apply, id->args.tokens.src1, NULL, ret_v);
			
			TAC* arg_values = mmc_icg(ast->right);

			apply->next = arg_values;
			// Call function
			if (ast->right->type != 44){ // If just a single argument, we load it in a0
				TOKEN* arg = (TOKEN*)malloc(sizeof(TOKEN));
				arg->lexeme = malloc(sizeof(char)*5);
				sprintf(arg->lexeme, "$a%d", args_count++);
				TAC* move_result = new_tac(tac_move, arg_values->args.tokens.dst, NULL, arg);
				attach_tac(arg_values, move_result);
			}

			
			TAC* call = new_tac(tac_call, NULL, NULL, id->args.tokens.src1);
			attach_tac(arg_values, call);
			return apply;
		case 44:; // (,)
			// Process multiple arguments for functions
			TAC* left_args = mmc_icg(ast->left);
			TOKEN* arg1 = (TOKEN*)malloc(sizeof(TOKEN));
			arg1->lexeme = malloc(sizeof(char)*5);
			sprintf(arg1->lexeme, "$a%d", args_count++);
			TAC* move_result_left = new_tac(tac_move, left_args->args.tokens.dst, NULL, arg1);
			attach_tac(left_args, move_result_left);

			TAC* right_args = mmc_icg(ast->right);
			TOKEN* arg2 = (TOKEN*)malloc(sizeof(TOKEN));
			arg2->lexeme = malloc(sizeof(char)*5);
			sprintf(arg2->lexeme, "$a%d", args_count++);
			TAC* move_result_right = new_tac(tac_move, right_args->args.tokens.dst, NULL, arg2);
			attach_tac(right_args, move_result_right);

			attach_tac(left_args, right_args);
			return left_args;
		default:
			printf("unknown type code %d (%p) in mmc_icg\n",ast->type,ast);
			return NULL;
  	}
}

// Create a new basic block
BB* new_basic_block(TAC* tac){
	BB* bb = (BB*)malloc(sizeof(BB));
	bb->leader = tac;
	bb->next = NULL;
	return bb;
}

// Generate a graph of basic blocks that splits up the TAC depending on certain condition defined in the spec
BB* block_graph_gen(TAC* tac){
	if (tac != NULL && (tac->op == tac_proc || tac->op == tac_label)){
		BB* bb = new_basic_block(tac);
		bb->next = block_graph_gen(tac->next);
		return bb;
	} else if (tac != NULL){
		return block_graph_gen(tac->next);
	} else {
		return NULL;
	}
}

// Print out the blocks seperately
void print_blocks(BB* bb){
	int x = 0;
	
	for(TAC* i = bb->leader; i != NULL; i = i->next){
		if (bb == NULL){
			print_single_tac(i);
		} else if (i == bb->leader){
			printf("\nBasic Block %d\n", x);
			x++;
			bb = bb->next;
			print_single_tac(i);
		} else {
			print_single_tac(i);
		}
	}
}

// Optimise a block
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
			if (load1->op == tac_load && 
				load2->op == tac_load && 
				load1->args.tokens.dst->value == load2->args.tokens.dst->value){
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
		} else if (i->op != tac_proc && i->op != tac_proc_end){
			push(stack, (void*)i);
		}
	}
	printf("\nOptimised:\n");
	mmc_print_ic(leader);
}

// Print a single TAC for debugging purposes
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
	} else if (i->op == tac_return || i->op == tac_label || i->op == tac_goto || i->op == tac_call){
		printf("%s %s\n",
		tac_ops[i->op],
		i->args.tokens.dst->lexeme);
	} else if (i->op == tac_apply) {
		printf("%s %s %s\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.dst->lexeme);
	} else if (i->op == tac_store_word || i->op == tac_move){
		if (i->args.tokens.src1 != NULL){
			printf("%s %s, %s\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.dst->lexeme);
		} else {
			printf("%s %d, %s\n", tac_ops[i->op], 0, i->args.tokens.dst->lexeme);
		}
	} else if (i->op == tac_proc){
		printf("%s %s %d\n", tac_ops[i->op], i->args.call->name->lexeme, i->args.call->arity);
	} else if (i->op == tac_load_word){
		printf("%s %s, %s\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.dst->lexeme);
	} else if (i->op == tac_proc_end || i->op == tac_main_end){
		printf("%s\n\n", tac_ops[i->op]);
	} else if (i->op == tac_arg){
		printf("%s %s, %d\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.dst->value);
	} else if(i->op == tac_equality){
		printf("%s %s == %s %s\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.src2->lexeme, i->args.tokens.dst->lexeme);
	} else if (i->op == tac_n_equality){
		printf("%s %s != %s %s\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.src2->lexeme, i->args.tokens.dst->lexeme);
	} else if (i->op == tac_gt_op){
		printf("%s %s > %s %s\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.src2->lexeme, i->args.tokens.dst->lexeme);
	}  else if (i->op == tac_lt_op){
		printf("%s %s < %s %s\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.src2->lexeme, i->args.tokens.dst->lexeme);
	} else if (i->op == tac_ge_op){
		printf("%s %s >= %s %s\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.src2->lexeme, i->args.tokens.dst->lexeme);
	} else if (i->op == tac_le_op){
		printf("%s %s <= %s %s\n", tac_ops[i->op], i->args.tokens.src1->lexeme, i->args.tokens.src2->lexeme, i->args.tokens.dst->lexeme);
	}
}

// Print out all the TACs
void mmc_print_ic(TAC* i)
{
  	for(;i!=NULL;i=i->next)
	print_single_tac(i);
}