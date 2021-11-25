#include "token.h"
#include "nodes.h"

enum valuetype {
	mmcINT = 1,
	mmcBOOL = 2,
	mmcSTRING = 3,
	mmcFUNC = 4
};

typedef struct value {
	int type;
	int is_func_ret;
	union {
		int integer;
		int boolean; // will need this soon
		char* string;
		void* function;
	} v;
} VALUE;

typedef struct binding {
	TOKEN* name;
	VALUE* val;
	struct binding* next;
} BINDING;

typedef struct frame {
	BINDING* binding;
	struct frame* next;
} FRAME;

typedef struct closure {
	FRAME* frame;
	NODE* code;
	NODE* body;
	NODE* args;
} CLOSURE;

FRAME* new_frame();
BINDING* new_binding(NODE* name, VALUE* val, BINDING* next);
FRAME* extend_frame(FRAME* env, NODE* ids, NODE* args);
VALUE* lexical_call_method(TOKEN* name, NODE* args, FRAME* frame);
CLOSURE* new_closure(NODE* func, FRAME* frame);

VALUE* add_values(VALUE* left_operand, VALUE* right_operand);
VALUE* sub_values(VALUE* left_operand, VALUE* right_operand);
VALUE* div_values(VALUE* left_operand, VALUE* right_operand);
VALUE* mul_values(VALUE* left_operand, VALUE* right_operand);
VALUE* mod_values(VALUE* left_operand, VALUE* right_operand);

VALUE* declare_variable(TOKEN* var, FRAME* frame);
VALUE* declare_function(NODE* func, FRAME* frame);
VALUE* get_variable(TOKEN* var, FRAME* frame);
VALUE* new_value(int type, void* value);
VALUE* equality_calculator(int type, NODE* tree, FRAME* frame);

VALUE* interpret(NODE *tree, FRAME* frame);