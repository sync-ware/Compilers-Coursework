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
   tac_plus = 1
  };

typedef struct tac {
int op ;
TOKEN* src1;
TOKEN* src2;
TOKEN* dst;
struct tac* next;
} TAC;

int availableAddresses = 8;
#define MAX_ADDRESSES 8


char int_to_char(int x)
{
  char ret = (x + 48);
  return ret;
}

TAC* new_tac(int op, TOKEN* src1, TOKEN* src2, TOKEN* dst)
{
  TAC* ans = (TAC*)malloc(sizeof(TAC));
  if (ans==NULL) {
    printf("Error! memory not allocated.");
    exit(0);
  }
  ans->op = op;
  char address_num;
  char address[4] = "$t";
  if (availableAddresses > 0){
    address_num = int_to_char(MAX_ADDRESSES-availableAddresses);
    strcpy(src1->lexeme, address);
    strncat(src1->lexeme, &address_num, 1);
    availableAddresses--;
    address_num = int_to_char(MAX_ADDRESSES-availableAddresses);
    strcpy(src2->lexeme, address);
    strncat(src2->lexeme, &address_num, 1);
    availableAddresses--;
  }
  ans->src1 = src1;
  ans->src2 = src2;
  if (dst->lexeme == NULL){
    if (availableAddresses > 0){
      address_num = int_to_char(MAX_ADDRESSES-availableAddresses);
      strcpy(dst->lexeme, address);
      strncat(dst->lexeme, &address_num, 1);
      availableAddresses--;
    }
  }
  ans->dst = dst;
}


TAC* mmc_icg(NODE* ast)
{
  switch (ast->type) {
    case 68: //D
      printf("Begin Interpretation.\n");
      return mmc_icg(ast->right);
    break;
    case RETURN:
      printf("Return found.\n");
      return mmc_icg(ast->left); 
    break;
    case LEAF:
      printf("Leaf found.\n");
      return mmc_icg(ast->left);
    break;
    case 43: //+
      printf("Plus found.\n");
      return new_tac(tac_plus, (TOKEN*) ast->left, (TOKEN*) ast->right, (TOKEN*) ast->left)
      break;
    // case 45: //-
    //   printf("Minus found.\n");
    //   return minusValues(interpret(ast->left), interpret(ast->right));
    //   break;
    // case 47: //(/)
    //   printf("Divide found.\n");
    //   return divideValues(interpret(ast->left), interpret(ast->right));
    //   break;
    // case 42: //(*)
    //   printf("Multiplication found.\n");
    //   return multiplyValues(interpret(ast->left), interpret(ast->right));
    //   break;
    // case 37: //%
    //   printf("Modulo found.\n");
    //   return moduloValues(interpret(ast->left), interpret(ast->right));
    //   break;
    // case CONSTANT:;
    //   TOKEN *t = (TOKEN *)ast;
    //   printf("Constant found: %d.\n",t->value);
    //   VALUE* value = (VALUE*)malloc(sizeof(VALUE));;
    //   value->type = mmcINT;
    //   value->v.integer = t->value;
    //   return value;

    // break;
    default:
    printf("unknown type code %d (%p) in mmc_icg\n",ast->type,ast);
    return NULL;
  }
};

char* tac_ops[] = {"NOOP","ADD"};

void mmc_print_ic(TAC* i)
{
  for(;i!=NULL;i=i->next)
    printf("%s %s, %s, %s\n",
	   tac_ops[i->op], // need to range check!
	   i->src1->lexeme,
	   i->src2->lexeme,
	   i->dst->lexeme);
}

typedef struct value {
  int          type;
  union {
    int integer;
    int boolean; // will need this soon
    char* string;
    void* function;
  } v;
} VALUE;

enum valuetype
  {
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

int findArg(int argc, char** argv, char* elem)
{
  for(int x = 1; x < argc; x++) {
    if (strcmp(argv[x], elem) == 0) {
      return 1;
    }
  }
  return 0;
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
    if (findArg(argc, argv, "-i"))
    {
      VALUE* status = interpret(tree);
      printf("Program exited with status code %d.\n", status->v.integer);
      
    }
    if (findArg(argc, argv, "-m"))
    {
      TAC* tac = mmc_icg(tree);
      mmc_print_ic(tac);
      
    }
    return 0;
}
