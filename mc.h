#include "tac.h"

typedef struct mc {
  char* insn;
  struct mc* next;
} MC;

MC* new_mci(char* s);
MC* mmc_mcg(TAC* i);
int find_word(char* words[], char* word, int length);
MC* gen_words(int x);
void attach_ins(MC* left, MC* right);
MC* three_address_generate(char* op, TAC* i);
void mmc_print_mc(MC* i);