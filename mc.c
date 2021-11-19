#include "mc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

MC* three_address_generate(char* op, TAC* i){
	char str[50];
	strncat(str, op, 5);
	strncat(str, i->dst->lexeme, strlen(i->dst->lexeme)+1);
	strncat(str, ", ", 3);
	strncat(str, i->src1->lexeme, strlen(i->dst->lexeme)+1);
	strncat(str, ", ", 3);
	strncat(str, i->src2->lexeme, strlen(i->dst->lexeme)+1);
	MC* ins = new_mci(str);
	ins->next = mmc_mcg(i->next);
	return ins;
}

MC* mmc_mcg(TAC* i){
	if (i==NULL) return NULL;
	switch (i->op) {
		case tac_plus:
			return three_address_generate("add ", i);

		case tac_load:;
			char str_load[50] = "li ";
			strncat(str_load, i->dst->lexeme, strlen(i->dst->lexeme)+1);
			strncat(str_load, ", ", 3);
			char raw_val[8]; 
			sprintf(raw_val, "%d", i->dst->value);
			strncat(str_load, raw_val, 8);
			MC* ins_load = new_mci(str_load);
			ins_load->next = mmc_mcg(i->next);
			return ins_load;

		case tac_minus:;
			return three_address_generate("sub ", i);

		case tac_multiply:
			return three_address_generate("mul ", i);

		case tac_divide:;
			char str_div[50] = "div ";
			strncat(str_div, i->src1->lexeme, strlen(i->src1->lexeme)+1);
			strncat(str_div, ", ", 3);
			strncat(str_div, i->src2->lexeme, strlen(i->src2->lexeme)+1);
			strncat(str_div, "\n", 2);
			strncat(str_div, "mflo ", 6); // Qoutient goes into register $hi, so we need to move it into destination
			strncat(str_div, i->dst->lexeme, strlen(i->dst->lexeme)+1);
			MC* ins_div = new_mci(str_div);
			ins_div->next = mmc_mcg(i->next);
			return ins_div;
		
		case tac_mod:;
			char str_mod[50] = "div ";
			strncat(str_mod, i->src1->lexeme, strlen(i->src1->lexeme)+1);
			strncat(str_mod, ", ", 3);
			strncat(str_mod, i->src2->lexeme, strlen(i->src2->lexeme)+1);
			strncat(str_mod, "\n", 2);
			strncat(str_mod, "mfhi ", 6); // Remainder goes into register $lo, so we need to move it into destination
			strncat(str_mod, i->dst->lexeme, strlen(i->dst->lexeme)+1);
			MC* ins_mod = new_mci(str_mod);
			ins_mod->next = mmc_mcg(i->next);
			return ins_mod;

		case tac_return:
			// char str_ret[50] = "move $v0, ";
			// strncat(str_ret, i->dst->lexeme, strlen(i->dst->lexeme)+1);
			// MC* ins_ret = new_mci(str_ret);
			// return ins_ret;
			return new_mci(""); // Temporary

		case tac_proc:;
			char str_proc[50];
			strncat(str_proc, i->dst->lexeme, strlen(i->dst->lexeme)+1);
			strncat(str_proc, ":\n", 3);
			MC* ins_proc = new_mci(str_proc);
			ins_proc->next = mmc_mcg(i->next);
			return ins_proc;

		// case tac_assign:
		// 	char str_assign[50] = "sw ";
		default:
			printf("unknown type code %d (%p) in mmc_mcg\n",i->op,i);
			return NULL;
	}
}

void mmc_print_mc(MC* i){
  	for(;i!=NULL;i=i->next) printf("%s\n",i->insn);
}