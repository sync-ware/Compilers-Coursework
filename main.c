#include <stdio.h>
#include <ctype.h>
#include "nodes.h"
#include "C.tab.h"
#include <string.h>
#include <stdlib.h>

char *named(int t)
{
    static char b[100];
    if (isgraph(t) || t==' ') {
      sprintf(b, "%c", t);
      return b;
    }
    switch (t) {
      default: return "???";
    case IDENTIFIER:
      return "id";
    case CONSTANT:
      return "constant";
    case STRING_LITERAL:
      return "string";
    case LE_OP:
      return "<=";
    case GE_OP:
      return ">=";
    case EQ_OP:
      return "==";
    case NE_OP:
      return "!=";
    case EXTERN:
      return "extern";
    case AUTO:
      return "auto";
    case INT:
      return "int";
    case VOID:
      return "void";
    case APPLY:
      return "apply";
    case LEAF:
      return "leaf";
    case IF:
      return "if";
    case ELSE:
      return "else";
    case WHILE:
      return "while";
    case CONTINUE:
      return "continue";
    case BREAK:
      return "break";
    case RETURN:
      return "return";
    }
}

void print_leaf(NODE *tree, int level)
{
    TOKEN *t = (TOKEN *)tree;
    int i;
    for (i=0; i<level; i++) putchar(' ');
    if (t->type == CONSTANT) printf("%d\n", t->value);
    else if (t->type == STRING_LITERAL) printf("\"%s\"\n", t->lexeme);
    else if (t) puts(t->lexeme);
}

void print_tree0(NODE *tree, int level)
{
    int i;
    if (tree==NULL) return;
    if (tree->type==LEAF) {
      print_leaf(tree->left, level);
    }
    else {
      for(i=0; i<level; i++) putchar(' ');
      printf("%s\n", named(tree->type));
/*       if (tree->type=='~') { */
/*         for(i=0; i<level+2; i++) putchar(' '); */
/*         printf("%p\n", tree->left); */
/*       } */
/*       else */
        print_tree0(tree->left, level+2);
      print_tree0(tree->right, level+2);
    }
}

void print_tree(NODE *tree)
{
    print_tree0(tree, 0);
}

extern int yydebug;
extern NODE* yyparse(void);
extern NODE* ans;
extern void init_symbtable(void);

enum tac_op
  {
   tac_plus = 1,
   tac_load = 2,
   tac_return = 3,
   tac_minus = 4,
   tac_divide = 5,
   tac_multiply = 6
  };

char* tac_ops[] = {"NOOP","ADD", "LOAD", "RETURN", "SUBTRACT", "DIVIDE", "MULTIPLY"};

typedef struct tac {
int op ;
TOKEN* src1;
TOKEN* src2;
TOKEN* dst;
struct tac* next;
} TAC;


typedef struct mc {
  char* insn;
  struct mc* next;
} MC;

int availableAddresses = 8;
#define MAX_ADDRESSES 8


char int_to_char(int x)
{
  char ret = (x + 48);
  return ret;
}

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

		default:
			return NULL;
  	}

}

void mmc_print_ic(TAC* i)
{
  	for(;i!=NULL;i=i->next)
	if (i->op == tac_plus || i->op == tac_minus || i->op == tac_divide || i->op == tac_multiply){
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
		// case 37: //%
		//   printf("Modulo found.\n");
		//   return moduloValues(interpret(ast->left), interpret(ast->right));
		//   break;
		default:
			printf("unknown type code %d (%p) in mmc_icg\n",ast->type,ast);
			return NULL;
  	}
};

typedef struct value {
  int          type;
  union {
    int integer;
    int boolean; // will need this soon
    char* string;
    void* function;
  } v;
} VALUE;

enum valuetype {
   mmcINT = 1,
   mmcBOOL = 2,
   mmcSTRING = 3
};

VALUE* addValues(VALUE* leftValue, VALUE* rightValue){
  leftValue->v.integer = leftValue->v.integer + rightValue->v.integer;
  free(rightValue);
  return leftValue;
}

VALUE* minusValues(VALUE* leftValue, VALUE* rightValue){
  leftValue->v.integer = leftValue->v.integer - rightValue->v.integer;
  free(rightValue);
  return leftValue;
}

VALUE* divideValues(VALUE* leftValue, VALUE* rightValue){
  leftValue->v.integer = leftValue->v.integer / rightValue->v.integer;
  free(rightValue);
  return leftValue;
}

VALUE* multiplyValues(VALUE* leftValue, VALUE* rightValue){
  leftValue->v.integer = leftValue->v.integer * rightValue->v.integer;
  free(rightValue);
  return leftValue;
}

VALUE* moduloValues(VALUE* leftValue, VALUE* rightValue){
  leftValue->v.integer = leftValue->v.integer % rightValue->v.integer;
  free(rightValue);
  return leftValue;
}

VALUE* interpret(NODE *tree)
{
  switch(tree->type){
    case 68: //D
      printf("Begin Interpretation.\n");
      return interpret(tree->right);
    break;
    case RETURN:
      printf("Return found.\n");
      return interpret(tree->left); 
    break;
    case LEAF:
      printf("Leaf found.\n");
      return interpret(tree->left);
    break;
    case 43: //+
      printf("Plus found.\n");
      return addValues(interpret(tree->left), interpret(tree->right));
      break;
    case 45: //-
      printf("Minus found.\n");
      return minusValues(interpret(tree->left), interpret(tree->right));
      break;
    case 47: //(/)
      printf("Divide found.\n");
      return divideValues(interpret(tree->left), interpret(tree->right));
      break;
    case 42: //(*)
      printf("Multiplication found.\n");
      return multiplyValues(interpret(tree->left), interpret(tree->right));
      break;
    case 37: //%
      printf("Modulo found.\n");
      return moduloValues(interpret(tree->left), interpret(tree->right));
      break;
    case CONSTANT:;
      TOKEN *t = (TOKEN *)tree;
      printf("Constant found: %d.\n",t->value);
      VALUE* value = (VALUE*)malloc(sizeof(VALUE));;
      value->type = mmcINT;
      value->v.integer = t->value;
      return value;
    default:
    break;
  }
}


MC* new_mci(char* s)
{
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

MC* mmc_mcg(TAC* i)
{
	if (i==NULL) return NULL;
	switch (i->op) {
		case tac_plus:;
			char str[50] = "add ";
			strncat(str, i->dst->lexeme, strlen(i->dst->lexeme)+1);
			strncat(str, ", ", 3);
			strncat(str, i->src1->lexeme, strlen(i->dst->lexeme)+1);
			strncat(str, ", ", 3);
			strncat(str, i->src2->lexeme, strlen(i->dst->lexeme)+1);
			//strncat(str, "\0", 1);
			MC* ins = new_mci(str);
			ins->next = mmc_mcg(i->next);
			return ins;

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

		case tac_return:;
			char str_ret[50] = "move $v0, ";
			strncat(str_ret, i->dst->lexeme, strlen(i->dst->lexeme)+1);
			MC* ins_ret = new_mci(str_ret);
			return ins_ret;
	default:
		printf("unknown type code %d (%p) in mmc_mcg\n",i->op,i);
		return NULL;
	}
}


void mmc_print_mc(MC* i)
{
  	for(;i!=NULL;i=i->next) printf("%s\n",i->insn);
}

int findArg(int argc, char** argv, char* elem)
{
  for(int x = 1; x < argc; x++) {
    if (strcmp(argv[x], elem) == 0) {
      return 1;
    }
  }
  return 0;
}

void write_to_file(MC* i){
	FILE* fptr;
	fptr = fopen("output.asm", "w");
	fprintf(fptr, "%s\n", ".globl main\n.text\nmain:");
	for(;i!=NULL;i=i->next) {
		fprintf(fptr,"%s\n",i->insn);
	}
	fprintf(fptr, "%s\n", "syscall");
	fclose(fptr);
}

int main(int argc, char** argv)
{
    NODE* tree;
    if (findArg(argc, argv, "-d")) yydebug = 1;
    init_symbtable();
    printf("--C COMPILER\n");
    yyparse();
    tree = ans;
    printf("parse finished with %p\n", tree);
    print_tree(tree);
    if (findArg(argc, argv, "-i")) {
      VALUE* status = interpret(tree);
      printf("Program exited with status code %d.\n", status->v.integer);
      
    }
    if (findArg(argc, argv, "-m")) {
		TAC* tac = mmc_icg(tree);
		mmc_print_ic(tac);

		if (findArg(argc, argv, "-a")) { //Assembly
			printf("\n");
			MC* mc = mmc_mcg(tac);
			mmc_print_mc(mc);
			write_to_file(mc);
		}
    }

    return 0;
}
