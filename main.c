#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "tac.h"
#include "C.tab.h"
#include "interpreter.h"
#include "mc.h"

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
	fprintf(fptr, "%s\n", ".globl main\n.text\n");
	for(;i!=NULL;i=i->next) {
		fprintf(fptr,"%s\n",i->insn);
	}
	//fprintf(fptr, "%s\n", "li $v0, 10\nsyscall");
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
		FRAME* frame = new_frame();
		printf("\n");
    
		VALUE* interpretation = interpret(tree, frame);
    TOKEN* main = new_token(mmcINT);
    main->lexeme = malloc(sizeof(char)*5);
    main->lexeme = "main";
    VALUE* status = lexical_call_method(main, NULL, frame);
    //printf("Status achieved\n");
		printf("Program exited with status code %d.\n\n", status->v.integer);
	}
	if (findArg(argc, argv, "-m")) {
		TAC* tac = mmc_icg(tree);
		printf("\n");
		mmc_print_ic(tac);
    printf("\n");
    BB* bb = block_graph_gen(tac);
    print_blocks(bb);
    //optimise_block(bb);

		if (findArg(argc, argv, "-a")) { //Assembly
			printf("\n");
			MC* mc = mmc_mcg(tac);
			mmc_print_mc(mc);
			write_to_file(mc);
		}
	}

    return 0;
}
