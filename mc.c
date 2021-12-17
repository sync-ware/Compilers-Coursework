#include "mc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// New MIPS instruction
MC* new_mci(char* s){
	MC* ans = (MC*)malloc(sizeof(MC));
	if (ans==NULL) {
		printf("Error! memory not allocated.");
		exit(0);
	}
	ans->insn = (char*)malloc(sizeof(char)*50);
	strcpy(ans->insn, s);
	ans->next = NULL;
	return ans;
}

// Generate an instruction that has three addresses
MC* three_address_generate(char* op, TAC* i){
	char str[50] = "";
	strncat(str, op, 5);
	strncat(str, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
	strncat(str, ", ", 3);
	strncat(str, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);
	strncat(str, ", ", 3);
	strncat(str, i->args.tokens.src2->lexeme, strlen(i->args.tokens.src2->lexeme)+1);
	MC* ins = new_mci(str);
	ins->next = mmc_mcg(i->next);
	return ins;
}

// Look for a variable word from a list of words
int find_word(char* words[], char* word, int length){
	for(int x = 0; x < length; x++){
		if (strcmp(word, words[x]) == 0){
			return 1;
		}
	}
	return 0;
}

// Attach two MC blocks
void attach_ins(MC* left, MC* right){
	if (left->next == NULL){
		left->next = right;
	} else {
		attach_ins(left->next, right);
	}
}

char* words[10]; // Max 10 variables at this moment
int word_count = 0; // Keep track of number of variables used

// Main MC generator, we just translate a TAC into zero or more MC instructions
MC* mmc_mcg(TAC* i){
	if (i==NULL) return NULL;
	char* str_ins = malloc(sizeof(char)*20);
	MC* ins;
	switch (i->op) {
		case tac_plus:
			return three_address_generate("add ", i);

		case tac_load:;
			strncat(str_ins, "li ", 4);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			strncat(str_ins, ", ", 3);
			char raw_val[8]; 
			sprintf(raw_val, "%d", i->args.tokens.dst->value);
			strncat(str_ins, raw_val, 9);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_minus:;
			return three_address_generate("sub ", i);

		case tac_multiply:
			return three_address_generate("mul ", i);

		case tac_divide:;
			strncat(str_ins, "div ", 5);
			strncat(str_ins, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.src2->lexeme, strlen(i->args.tokens.src2->lexeme)+1);
			strncat(str_ins, "\n", 2);
			strncat(str_ins, "mflo ", 6); // Qoutient goes into register $hi, so we need to move it into destination
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;
		
		case tac_mod:;
			strncat(str_ins, "div ", 5);
			strncat(str_ins, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.src2->lexeme, strlen(i->args.tokens.src2->lexeme)+1);
			strncat(str_ins, "\n", 2);
			strncat(str_ins, "mfhi ", 6); // Remainder goes into register $lo, so we need to move it into destination
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			ins= new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_return:
			strncat(str_ins, "move $v0, ", 11);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_proc:;
			char str_proc[10] = "";
			strncat(str_proc, i->args.call->name->lexeme, strlen(i->args.call->name->lexeme)+1);
			strncat(str_proc, ":", 2);
			ins = new_mci(str_proc);
			
			// Add variables and return address to stack
			if (strcmp(i->args.call->name->lexeme, "main") != 0){
				MC* curr = ins;
				for(int x = 0; x < i->args.call->arity; x++){
					MC* mc_stack = new_mci("addi $sp,$sp,-4");
					curr->next = mc_stack;
					char str_store[40];
					sprintf(str_store, "move $s%d, $a%d\nsw $s%d,0($sp)", x, x, x);
					curr = new_mci(str_store);
					mc_stack->next = curr;
				}
				MC* ret_addr = new_mci("addi $sp,$sp,-4\nsw $ra,0($sp)");
				curr->next = ret_addr;
				ret_addr->next = mmc_mcg(i->next);
				return ins;
			} else {
				// Don't worry about loading on to the stack with main
				ins->next = mmc_mcg(i->next);
				return ins;
			}

		case tac_load_word:;
			if (find_word(words, i->args.tokens.src1->lexeme, word_count) == 0){
				words[word_count] = i->args.tokens.src1->lexeme;
				word_count++;
			}

			strncat(str_ins, "lw ", 4);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_store_word:;
			strncat(str_ins, "sw ", 4);
			strncat(str_ins, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;
		
		case tac_equality:;
			strncat(str_ins, "beq ", 5);
			strncat(str_ins, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.src2->lexeme, strlen(i->args.tokens.src2->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_n_equality:;
			strncat(str_ins, "bne ", 5);
			strncat(str_ins, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.src2->lexeme, strlen(i->args.tokens.src2->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_gt_op:;
			strncat(str_ins, "bgt ", 5);
			strncat(str_ins, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.src2->lexeme, strlen(i->args.tokens.src2->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_ge_op:;
			strncat(str_ins, "bge ", 5);
			strncat(str_ins, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.src2->lexeme, strlen(i->args.tokens.src2->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_lt_op:;
			strncat(str_ins, "blt ", 5);
			strncat(str_ins, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.src2->lexeme, strlen(i->args.tokens.src2->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_le_op:;
			strncat(str_ins, "ble ", 5);
			strncat(str_ins, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.src2->lexeme, strlen(i->args.tokens.src2->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_label:;
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			strncat(str_ins, ":", 2);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_goto:;
			strncat(str_ins, "j ", 3);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_proc_end:
			ins = new_mci("lw $ra,0($sp)\naddi $sp,$sp,4"); // Start with storing the return address on the stack
			
			MC* curr_ret = ins;
			// Load all the variables used and place them on the stack too
			for(int x = i->args.call->arity-1; x >=0 ; x--){
				char str_retrieve[50] = "";
				sprintf(str_retrieve, "lw $s%d,0($sp)\nsw $s%d, ", x, x);
				
				strncat(str_retrieve, i->args.call->arg_names[x], strlen(i->args.call->arg_names[x])+1);
				strncat(str_retrieve, "\naddi $sp,$sp,4", 16);
				MC* mc_retrieve = new_mci(str_retrieve);
				curr_ret->next = mc_retrieve;
				curr_ret = mc_retrieve;
			}
			
			MC* jump_return = new_mci("jr $ra"); // Jump back to return address
			curr_ret->next = jump_return;
			jump_return->next = mmc_mcg(i->next);
			return ins;

		case tac_main_end:
			return mmc_mcg(i->next); // skip

		case tac_arg:
			strncat(str_ins, "li ", 4);
			strncat(str_ins, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);
			strncat(str_ins, ", ", 3);
			char raw_val_arg[8]; 
			sprintf(raw_val_arg, "%d", i->args.tokens.dst->value);
			strncat(str_ins, raw_val_arg, 9);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_call:
			strncat(str_ins, "jal ", 5);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;

		case tac_move:
			strncat(str_ins, "move ", 6);
			strncat(str_ins, i->args.tokens.dst->lexeme, strlen(i->args.tokens.dst->lexeme)+1);
			strncat(str_ins, ", ", 3);
			strncat(str_ins, i->args.tokens.src1->lexeme, strlen(i->args.tokens.src1->lexeme)+1);

			ins = new_mci(str_ins);
			ins->next = mmc_mcg(i->next);
			return ins;
		case tac_apply:
			return mmc_mcg(i->next);
		default:
			printf("unknown type code %d (%p) in mmc_mcg\n",i->op,i);
		return NULL;
	}

}

// Generate the words that are used in the program
MC* gen_words(int x){
	char word_str[30] = "";
	strncat(word_str, words[x], strlen(words[x])+1);
	strncat(word_str, ": .word 0", 11);
	MC* word = new_mci(word_str);
	
	if (x == word_count-1){
		
		return word;
	} else {
		word->next = gen_words(x+1);
		return word;
	}
}

// Print out the MC instructions along with required MC
void mmc_print_mc(MC* i){
	MC* success_exit = new_mci("li $v0, 10");
	MC* exit = new_mci("syscall");
	success_exit->next = exit;
	attach_ins(i, success_exit);
	if (word_count > 0){
		MC* start_word = gen_words(0);
		exit->next = start_word;
	}
  	for(;i!=NULL;i=i->next) printf("%s\n",i->insn);
}