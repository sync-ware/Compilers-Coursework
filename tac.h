#ifndef __TAC_H
#define __TAC_H

#include "token.h"
#include "nodes.h"

#define MAX_ADDRESSES 16

enum tac_op {
    tac_noop = 0,
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
    tac_load_word = 14,
    tac_move = 15,
    tac_equality = 16,
    tac_n_equality = 17,
    tac_le_op = 18,
    tac_ge_op = 19,
    tac_gt_op = 20,
    tac_lt_op = 21,
    tac_label = 22,
    tac_goto = 23
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
    "LOAD WORD",
    "MOVE",
    "IF",
    "IF",
    "IF",
    "IF",
    "IF",
    "IF",
    "LABEL",
    "GO TO"
};

typedef struct call{
    TOKEN* name;
    int arity;
} CALL;

typedef struct block{
    int nvars;
} BLOCK;

typedef struct tac_tokens{
    TOKEN* src1;
    TOKEN* src2;
    TOKEN* dst;
} TAC_TOKENS;

typedef struct tac {
    int op ;
    union {BLOCK block; CALL call; TAC_TOKENS tokens;} args;
    struct tac* next;
} TAC;

typedef struct bb {
    TAC* leader;
    struct bb* next;
} BB;

TAC* new_tac(int op, TOKEN* src1, TOKEN* src2, TOKEN* dst);
TAC* new_proc_tac(int op, TOKEN* name, int arity);
void attach_tac(TAC* left, TAC* right);
TAC* arithmetic_tac(NODE* ast, int op);
int count_args(NODE* args);
TAC* conditonal_tac(NODE* ast, int type);
TAC* mmc_icg(NODE* ast);
void mmc_print_ic(TAC* i);
BB* new_basic_block(TAC* tac);
BB* block_graph_gen(TAC* tac);
void optimise_block(BB* bb);
void print_blocks(BB* bb);
void print_single_tac(TAC* i);
TOKEN* generate_label();

#endif