extern TOKEN* lookup_token(char *s);

/*
------------------------------------------------------------------------
interpretation
------------------------------------------------------------------------
*/

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

VALUE* new_int(int v) {
  VALUE* ans = (VALUE*)malloc(sizeof(VALUE));
  if (ans==NULL) {
    printf("Error! memory not allocated.");
    exit(0);
  }
  ans->type = mmcINT;
  ans->v.integer = v;
  return ans;
}

void mmc_print(VALUE* v) {
  if (v==NULL) return;
  switch (v->type) {
  case mmcINT:
    printf("%d\n",v->v.integer);
    break;
  case mmcBOOL:
  case mmcSTRING:
  default:
    printf("unknown type code %d (%p) in mmc_print\n",v->type,v);
  }
}

VALUE* mmc_interpret(NODE* ast)
{
  switch (ast->type) {
  default:
    printf("unknown type code %d (%p) in mmc_interpret\n",ast->type,ast);
    return NULL;
  }
};

/*
------------------------------------------------------------------------
intermdiate code generation
------------------------------------------------------------------------
*/

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
} TAC ;

TAC* new_tac(int op, TOKEN* src1, TOKEN* src2, TOKEN* dst)
{
  TAC* ans = (TAC*)malloc(sizeof(TAC));
  if (ans==NULL) {
    printf("Error! memory not allocated.");
    exit(0);
  }
  ans->op = op;
  ans->src1 = src1;
  ans->src2 = src2;
  ans->dst = dst;
  return ans;
}

TAC* mmc_icg(NODE* ast)
{
  switch (ast->type) {
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

/*
------------------------------------------------------------------------
machine code generation
------------------------------------------------------------------------
*/

typedef struct mc {
  char* insn;
  struct mc* next;
} MC;

MC* mmc_mcg(TAC* i)
{
  if (i==NULL) return NULL;
  switch (i->op) {
  default:
    printf("unknown type code %d (%p) in mmc_mcg\n",i->op,i);
    return NULL;
  }
}

MC* new_mci(char* s)
{
  MC* ans = (MC*)malloc(sizeof(MC));
  if (ans==NULL) {
    printf("Error! memory not allocated.");
    exit(0);
  }
  ans->insn = s;
  ans->next = NULL;
  return ans;
}

void mmc_print_mc(MC* i)
{
  for(;i!=NULL;i=i->next) printf("%s\n",i->insn);
}

int main(int argc, char** argv)
{
    NODE* tree;
    VALUE* x;
    TAC* ic = NULL;
    TOKEN* t1;
    TOKEN* t2;
    TOKEN* t3;
    MC* mc = NULL;
    if (argc>1 && strcmp(argv[1],"-d")==0) yydebug = 1;
    init_symbtable();
    printf("--C COMPILER\n");
    yyparse();
    tree = ans;
    printf("parse finished with %p\n", tree);
    print_tree(tree);
    x = mmc_interpret(tree);
    // insert manual test cases here
    mmc_print(x);
    ic = mmc_icg(tree);
    // insert manual test cases here
    /* t1 = lookup_token("t1"); // needs extern declaration of lookup_token */
    /* t2 = lookup_token("t2"); */
    /* t3 = lookup_token("t3"); */
    /* ic = new_tac(tac_plus, t1, t2, t3); */
    mmc_print_ic(ic);
    mc = mmc_mcg(ic);
    // insert manual test cases here
    // mc = new_mci("abc");
    // mc->next = new_mci("def");
    mmc_print_mc(mc);
    printf("mmc finished\n");
    return 0;
}
