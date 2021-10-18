#include <stdio.h>
#include <ctype.h>
#include "nodes.h"
#include "C.tab.h"
#include <string.h>

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

int interpret(NODE *tree)
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
      return interpret(tree->left) + interpret(tree->right);
      break;
    case 45: //-
      printf("Minus found.\n");
      return interpret(tree->left) - interpret(tree->right);
      break;
    case 47: //(/)
      printf("Divide found.\n");
      return interpret(tree->left) / interpret(tree->right);
      break;
    case 42: //(*)
      printf("Multiplication found.\n");
      return interpret(tree->left) * interpret(tree->right);
      break;
    case 37: //%
      printf("Modulo found.\n");
      return interpret(tree->left) % interpret(tree->right);
      break;
    case CONSTANT:;
      TOKEN *t = (TOKEN *)tree;
      printf("Constant found: %d.\n",t->value);
      return t->value;
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
      int status = interpret(tree);
      printf("Program exited with status code %d.\n", status);
      
    }
    return 0;
}
