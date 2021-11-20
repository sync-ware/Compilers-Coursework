#ifndef __TAC_H
#define __TAC_H

#include "token.h"
#include "nodes.h"

#define MAX_ADDRESSES 16

enum tac_op {
	tac_plus = 1,
	tac_load = 2,
	tac_return = 3,
	tac_minus = 4,
	tac_divide = 5,
	tac_multiply = 6,
	tac_mod = 7,
	tac_store_word = 8,
	tac_declare = 9,
	tac_variable = 10,
    tac_proc = 11,
    tac_arg = 12,
    tac_proc_end = 13,
    tac_load_word = 14
};

static char* tac_ops[] = {
    "NOOP",
    "ADD", 
    "LOAD", 
    "RETURN", 
    "SUBTRACT", 
    "DIVIDE", 
    "MULTIPLY", 
    "MOD", 
    "STORE WORD", 
    "DECLARE", 
    "VARIABLE",
    "PROC",
    "ARG",
    "END PROC",
    "LOAD WORD"
};

typedef struct tac {
    int op ;
    TOKEN* src1;
    TOKEN* src2;
    TOKEN* dst;
    struct tac* next;
} TAC;

TAC* new_tac(int op, TOKEN* src1, TOKEN* src2, TOKEN* dst);
void attach_tac(TAC* left, TAC* right);
TAC* arithmetic_tac(NODE* ast, int op);
TAC* mmc_icg(NODE* ast);
void mmc_print_ic(TAC* i);

#endif