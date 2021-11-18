#include "token.h"
#include "nodes.h"

enum valuetype {
   mmcINT = 1,
   mmcBOOL = 2,
   mmcSTRING = 3
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

typedef struct binding {
	TOKEN* name;
	VALUE* val;
	struct binding* next;
} BINDING;

typedef struct frame {
	BINDING* binding;
	struct frame* next;
} FRAME;

FRAME* new_frame();

VALUE* addValues(VALUE* leftValue, VALUE* rightValue);
VALUE* minusValues(VALUE* leftValue, VALUE* rightValue);
VALUE* divideValues(VALUE* leftValue, VALUE* rightValue);
VALUE* multiplyValues(VALUE* leftValue, VALUE* rightValue);
VALUE* moduloValues(VALUE* leftValue, VALUE* rightValue);

VALUE* declare_variable(TOKEN* var, FRAME* frame);
VALUE* get_variable(TOKEN* var, FRAME* frame);

VALUE* interpret(NODE *tree, FRAME* frame);