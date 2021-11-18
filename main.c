#include <stdio.h>
#include <ctype.h>
//#include "nodes.h"
//#include "C.tab.h"
#include <string.h>
#include <stdlib.h>
#include "tac.h"
#include "global.h"
#include "C.tab.h"

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

enum valuetype {
   mmcINT = 1,
   mmcBOOL = 2,
   mmcSTRING = 3
};

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



typedef struct value {
  int          type;
  union {
    int integer;
    int boolean; // will need this soon
    char* string;
    void* function;
  } v;
} VALUE;




typedef struct mc {
  char* insn;
  struct mc* next;
} MC;

typedef struct binding {
	TOKEN* name;
	VALUE* val;
	struct binding* next;
} BINDING;

typedef struct frame {
	BINDING* binding;
	struct frame* next;
} FRAME;

FRAME* new_frame(){
	FRAME* frame = (FRAME*)malloc(sizeof(FRAME));
	frame->binding = NULL;
	
	return frame;
}



char int_to_char(int x)
{
  char ret = (x + 48);
  return ret;
}



void mmc_print_ic(TAC* i)
{
  	for(;i!=NULL;i=i->next)
	if (i->op == tac_plus || i->op == tac_minus || i->op == tac_divide || i->op == tac_multiply || i->op == tac_mod){
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
	} else if (i->op == tac_declare){
		printf("%s %s\n", tac_ops[i->op], i->dst->lexeme);
	} else if (i->op == tac_assign){
		printf("%s %s, %s\n", tac_ops[i->op], i->src1->lexeme, i->dst->lexeme);
	}
}

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

VALUE* declare_variable(TOKEN* var, FRAME* frame){
	//printf("Variable declare\n");
	BINDING* bindings = frame->binding;
	BINDING* new = (BINDING*)malloc(sizeof(BINDING));
	if (new != 0){
		VALUE* value = (VALUE*)malloc(sizeof(VALUE));
		value->type = mmcINT;
		value->v.integer = var->value;
		new->name = var;
		new->val = value;
		new->next = bindings;
		frame->binding = new;
		return (VALUE*)0;
	}
	//error("Binding failed\n");
}

VALUE* get_variable(TOKEN* var, FRAME* frame){
	while (frame != NULL){
		BINDING* bindings = frame->binding;
		while (bindings != NULL){
			//printf("Var: %s, Binding: %s, Result: %d\n",var->lexeme, bindings->name->lexeme, strcmp(bindings->name->lexeme, var->lexeme));
			if (strcmp(bindings->name->lexeme, var->lexeme) == 0){
				return bindings->val;
			}
			bindings = bindings->next;
		}
		frame = frame->next;
	}
	return NULL;
}

VALUE* interpret(NODE *tree, FRAME* frame)
{
  	switch(tree->type){
    	case 68: //D
      		printf("Begin Interpretation.\n");
      		return interpret(tree->right, frame);
    	break;
    	case RETURN:
      		printf("Return found.\n");
      		return interpret(tree->left, frame); 
    	break;
		case LEAF:
			printf("Leaf found.\n");
			return interpret(tree->left, frame);
		break;
		case 43: //+
			printf("Plus found.\n");
			return addValues(interpret(tree->left, frame), interpret(tree->right, frame));
		break;
		case 45: //-
			printf("Minus found.\n");
			return minusValues(interpret(tree->left, frame), interpret(tree->right, frame));
		break;
		case 47: //(/)
			printf("Divide found.\n");
			return divideValues(interpret(tree->left, frame), interpret(tree->right, frame));
		break;
		case 42: //(*)
			printf("Multiplication found.\n");
			return multiplyValues(interpret(tree->left, frame), interpret(tree->right, frame));
		break;
		case 37: //%
			printf("Modulo found.\n");
			return moduloValues(interpret(tree->left, frame), interpret(tree->right, frame));
		break;
			case CONSTANT:;
			TOKEN *t = (TOKEN *)tree;
			printf("Constant found: %d.\n",t->value);
			VALUE* value = (VALUE*)malloc(sizeof(VALUE));;
			value->type = mmcINT;
			value->v.integer = t->value;
			return value;
		case 59: // ;
			printf("Sequence found\n");
			interpret(tree->left, frame); // Go through and interpret the first part of the sequence
			return interpret(tree->right, frame);	
		case ASSIGNMENT:
			printf("Assignment found\n");
			// Generate a token for the variable
			TOKEN* token = new_token(interpret(tree->left, frame)->v.integer); // Type
			token->lexeme = interpret(tree->right->left, frame)->v.string; // Variable name
			if (tree->right->right != NULL){ // Check to see if there is a value assigment.
				token->value = interpret(tree->right->right, frame)->v.integer; // Value
			}else{ // If no value, default to 0;
				token->value = 0;
			}

			return declare_variable(token, frame);
		case INT:
			printf("Int type found.\n");
			VALUE* int_value = (VALUE*)malloc(sizeof(VALUE));
			int_value->type=mmcINT;
			int_value->v.integer = mmcINT;
			return int_value;
		case IDENTIFIER:
			printf("Identifier found\n");
			
			TOKEN* id = (TOKEN*)tree;
			printf("Id: %s\n", id->lexeme);
			VALUE* found_id = get_variable(id, frame); // Check to see if it is already defined.
			if (found_id == NULL){
				printf("Make new variable\n");
				VALUE* id_val = (VALUE*)malloc(sizeof(VALUE));
				id_val->type = mmcSTRING;
				id_val->v.string = id->lexeme;
				return id_val;
			} else {
				printf("Variable found\n");
				return found_id;
			}
		case 61:
			printf("Equals found\n");
			VALUE* val = get_variable((TOKEN*)tree->left->left, frame);
			val->v.integer = interpret(tree->right, frame)->v.integer;
			return val;
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

MC* mmc_mcg(TAC* i);

MC* three_address_generate(char* op, TAC* i){
	char str[50];
	strncat(str, op, 5);
	strncat(str, i->dst->lexeme, strlen(i->dst->lexeme)+1);
	strncat(str, ", ", 3);
	strncat(str, i->src1->lexeme, strlen(i->dst->lexeme)+1);
	strncat(str, ", ", 3);
	strncat(str, i->src2->lexeme, strlen(i->dst->lexeme)+1);
	//strncat(str, "\0", 1);
	MC* ins = new_mci(str);
	ins->next = mmc_mcg(i->next);
	return ins;
}

MC* mmc_mcg(TAC* i)
{
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
	fprintf(fptr, "%s\n", "li $v0, 10\nsyscall");
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
		VALUE* status = interpret(tree, frame);
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
