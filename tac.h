#ifndef __TAC_H
#define __TAC_H

#include "token.h"
#include "nodes.h"
#include "stack.h"

#define MAX_ADDRESSES 9

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
    tac_goto = 23,
    tac_call = 24,
    tac_main_end = 25,
    tac_apply = 26
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
    "LOAD ARG",
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
    "GO TO",
    "CALL",
    "END MAIN",
    "APPLY"
};

typedef struct call{
    TOKEN* name;
    int arity;
    char** arg_names;
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
    union {BLOCK block; CALL* call; TAC_TOKENS tokens;} args;
    struct tac* next;
} TAC;

typedef struct bb {
    TAC* leader;
    struct bb* next;
} BB;

TAC* new_tac(int op, TOKEN* src1, TOKEN* src2, TOKEN* dst);
TOKEN* generate_label();
TAC* new_proc_tac(int op, TOKEN* name, STACK* stack);
TAC* new_end_proc_tac(int op, CALL* call);
void attach_tac(TAC* left, TAC* right);
TAC* end_tac(TAC* start);
TAC* arithmetic_tac(NODE* ast, int op);
void generate_args(NODE* args, STACK* stack);
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