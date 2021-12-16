#include "interpreter.h"
#include <stdlib.h>
#include <stdio.h>
#include "C.tab.h"
#include "global.h"
#include <string.h>

FRAME* new_frame(){
	FRAME* frame = (FRAME*)malloc(sizeof(FRAME));
	frame->binding = NULL;
	
	return frame;
}

// Create a new variable binding
BINDING* new_binding(NODE* name, VALUE* val, BINDING* next){
	BINDING* binding = (BINDING*)malloc(sizeof(BINDING));
	binding->name = (TOKEN*)name;
	binding->val = val;
	binding->next = next;
	return binding;
}

// Generate the bindings for a function
BINDING* gen_bindings(NODE* ids, NODE* args, FRAME* frame, BINDING* bindings){
	if (ids != NULL && ids->left->type != VOID){
		
		if (ids->type != 44){ //( , )
			bindings = new_binding(ids->right->left, interpret(args, frame), bindings);
		} else {
			bindings = gen_bindings(ids->left, args->left, frame, bindings);
			bindings = gen_bindings(ids->right, args->right, frame, bindings);
		}
		return bindings;
	} else {
		return NULL;
	}
}

// Extend a frame for a function
FRAME* extend_frame(FRAME* env, NODE* ids, NODE* args){
	FRAME* new_env = new_frame();
	
	BINDING* bindings = NULL;
	new_env->binding = gen_bindings(ids, args, env, bindings);
	return new_env;
}

// Call a function and supply it with arguments
VALUE* lexical_call_method(TOKEN* name, NODE* args, FRAME* frame){
	CLOSURE* f = (CLOSURE*)get_variable(name, frame)->v.function; // Retrieve the closure containing the function
	FRAME* new_env = extend_frame(frame, f->args, args); // New scope
	new_env->next = f->frame; // Link inner scope with closure frame
	return interpret(f->body, new_env); // Interpret the function body
}

// Arithmetic evaluation
VALUE* add_values(VALUE* left_operand, VALUE* right_operand){
	int calculation = left_operand->v.integer + right_operand->v.integer;
  	return new_value(mmcINT, (void*)&calculation);
}

VALUE* sub_values(VALUE* left_operand, VALUE* right_operand){
	int calculation = left_operand->v.integer - right_operand->v.integer;
	return new_value(mmcINT, (void*)&calculation);
}

VALUE* div_values(VALUE* left_operand, VALUE* right_operand){
  int calculation = left_operand->v.integer / right_operand->v.integer;
  return new_value(mmcINT, (void*)&calculation);
}

VALUE* mul_values(VALUE* left_operand, VALUE* right_operand){
  int calculation = left_operand->v.integer * right_operand->v.integer;
  return new_value(mmcINT, (void*)&calculation);
}

VALUE* mod_values(VALUE* left_operand, VALUE* right_operand){
  int calculation = left_operand->v.integer % right_operand->v.integer;
  return new_value(mmcINT, (void*)&calculation);
}

// Declare a new variable, bind it to the frame
VALUE* declare_variable(TOKEN* var, FRAME* frame){
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

// Similair to declare variable, except we must establish a closure and bind that to the frame
VALUE* declare_function(NODE* func, FRAME* frame){
	BINDING* bindings = frame->binding;
	BINDING* new = (BINDING*)malloc(sizeof(BINDING));
	if (new != 0){
		VALUE* value = (VALUE*)malloc(sizeof(VALUE));
		value->type = mmcFUNC;
		CLOSURE* closure = new_closure(func, frame);
		value->v.function = (void*)closure;
		TOKEN* token = new_token(mmcINT);
		token->lexeme = interpret(func->left->right->left, frame)->v.string;
		new->name = token;
		new->val = value;
		new->next = bindings;
		frame->binding = new;
		return new_value(mmcSTRING, (void*)&token->lexeme);
	}
}

// Create a new closure by breaking down the function tree
CLOSURE* new_closure(NODE* func, FRAME* frame){
	CLOSURE* closure = (CLOSURE*)malloc(sizeof(CLOSURE));
	closure->code = func;
	closure->frame = frame;
	closure->body = func->right;
	closure->args = func->left->right->right;
	return closure;
}

// Retrieve a variabel from the frame bindings
VALUE* get_variable(TOKEN* var, FRAME* frame){
	while (frame != NULL){
		BINDING* bindings = frame->binding;
		while (bindings != NULL){
			if (strcmp(bindings->name->lexeme, var->lexeme) == 0){
				return bindings->val;
			}
			bindings = bindings->next;
		}
		frame = frame->next;
	}
	//printf("Variable not found\n");
	return NULL;
}

// Generate a new VALUE
VALUE* new_value(int type, void* value){
	VALUE* val = (VALUE*)malloc(sizeof(VALUE));
	val->type = type;
	val->is_func_ret = 0;
	if (type == mmcINT || type == mmcBOOL){
		val->v.integer = *((int*)value);
	} else if (type == mmcSTRING){
		val->v.string = *((char**)value);
	}
	return val;
}

// Used to generate a boolean VALUE based on the conditional expression
VALUE* equality_calculator(int type, NODE* tree, FRAME* frame){
	VALUE* left_operand = interpret(tree->left, frame);
	VALUE* right_operand = interpret(tree->right, frame);
	int equality;
	if (left_operand->type == mmcINT && right_operand->type == mmcINT){
		switch(type){
			case EQ_OP:
				equality = left_operand->v.integer == right_operand->v.integer;
				break;
			case 62:
				equality = left_operand->v.integer > right_operand->v.integer;
				break;
			case 60:
				equality = left_operand->v.integer < right_operand->v.integer;
				break;
			case GE_OP:
				equality = left_operand->v.integer >= right_operand->v.integer;
				break;
			case LE_OP:
				equality = left_operand->v.integer <= right_operand->v.integer;
				break;
			case NE_OP:
				equality = left_operand->v.integer != right_operand->v.integer;
				break;
		}
		return new_value(mmcBOOL, (void*)&equality);
		
	}
	return NULL;
}

// Main interpret function
VALUE* interpret(NODE *tree, FRAME* frame)
{
  	switch(tree->type){
    	case 68: //D
			return declare_function(tree, frame);

    	case RETURN:;
			VALUE* ret = interpret(tree->left, frame);
			ret->is_func_ret = 1;
			return ret;
		case LEAF:
			return interpret(tree->left, frame);
		case 43: //+
			return add_values(interpret(tree->left, frame), interpret(tree->right, frame));
		case 45: //-
			return sub_values(interpret(tree->left, frame), interpret(tree->right, frame));
		case 47: //(/)
			return div_values(interpret(tree->left, frame), interpret(tree->right, frame));
		case 42: //(*)
			return mul_values(interpret(tree->left, frame), interpret(tree->right, frame));
		case 37: //%
			return mod_values(interpret(tree->left, frame), interpret(tree->right, frame));
		case CONSTANT:;
			TOKEN *t = (TOKEN *)tree;
			return new_value(mmcINT, (void*)&t->value);
		case 59:; // ;
			VALUE* left_seq = interpret(tree->left, frame); // Go through and interpret the first part of the sequence
			if (left_seq != NULL && left_seq->is_func_ret == 1){
				// If left sequnce has a return value, we need to return this
				return left_seq;
			}
			// Interpret right sequence
			VALUE* right_seq = interpret(tree->right, frame);
			return right_seq;
		case ASSIGNMENT:; // ~
			// If it's a function declaration, we need to do something else
			if (tree->left->type == ASSIGNMENT || tree->left->type == 68){
				// Process each function declaration seperately
				VALUE* left_branch = interpret(tree->left, frame);
				VALUE* right_branch = interpret(tree->right, frame);
				return NULL;
				
			} else {
				// Generate a token for the variable
				TOKEN* token = new_token(interpret(tree->left, frame)->v.integer); // Type
				token->lexeme = interpret(tree->right->left, frame)->v.string; // Variable name
				if (tree->right->right != NULL){ // Check to see if there is a value assigment.
					token->value = interpret(tree->right->right, frame)->v.integer; // Value
				}else{ // If no value, default to 0;
					token->value = 0;
				}
				return declare_variable(token, frame);
			}
		case INT:;
			int type = mmcINT;
			return new_value(mmcINT, (void*)&type);
		case IDENTIFIER:;
			// Retrieve a variable or return the token value so that it can be assigned a value
			TOKEN* id = (TOKEN*)tree;
			VALUE* found_id = get_variable(id, frame); // Check to see if it is already defined.
			if (found_id == NULL){
				return new_value(mmcSTRING, (void*)&id->lexeme);
			} else {
				return found_id;
			}
		case 61:; // =
			// Look for the variable and assign it a value based on the right sub-tree
			VALUE* val = get_variable((TOKEN*)tree->left->left, frame);
			val->v.integer = interpret(tree->right, frame)->v.integer;
			return val;
		case IF:;
			// Check if condition is true or not
			VALUE* condition = interpret(tree->left, frame);
			FRAME* new_scope = new_frame(); // Establish a new scope
			new_scope->next = frame;
			if (condition->v.boolean){
				if (tree->right->type == ELSE){
					// If body is in this part of the tree if ELSE exists
					return interpret(tree->right->left, new_scope);
				} else {
					// If body here if no ELSE exists
					return interpret(tree->right, new_scope);
				}
			} else {
				if (tree->right->type == ELSE){
					return interpret(tree->right, new_scope);
				} else {
					return NULL;
				}
			}
		case ELSE:
			return interpret(tree->right, frame);
		
		case EQ_OP: // ==
			return equality_calculator(EQ_OP, tree, frame);

		case 62: // >
			return equality_calculator(62, tree, frame);

		case 60: // <
			return equality_calculator(60, tree, frame);

		case GE_OP: // >=
			return equality_calculator(GE_OP, tree, frame);
		
		case LE_OP: // <=-
			return equality_calculator(LE_OP, tree, frame);

		case NE_OP: // (!=)
			return equality_calculator(NE_OP, tree, frame);
		case APPLY:;
			TOKEN* func_name;
			// If we have an apply within our apply, we need to resolve the inner apply first
			if (tree->left->type != APPLY){
				func_name = (TOKEN*)tree->left->left;
				// Check for built ins first
				if (strcmp(func_name->lexeme, "print_int") == 0){
					VALUE* print_value = interpret(tree->right, frame);
					printf("%d\n", print_value->v.integer);
					return NULL;
				} else if (strcmp(func_name->lexeme, "print_string") == 0) {
					VALUE* string_value = interpret(tree->right, frame);
					printf("%s", string_value->v.string);
					return NULL;
				} else if (strcmp(func_name->lexeme, "read_int") == 0) {
					int status, read_value = 0;
					do{
						status = scanf("%d", &read_value);
						int ch; //variable to read data into
						while((ch = getc(stdin)) != EOF && ch != '\n');
					}
					while (status == 0);

					return new_value(mmcINT, (void*)&read_value);
				} else {
					// If not a built in, call it.
					return lexical_call_method(func_name, tree->right, frame);
				}
			} else {
				// Resolve inner apply, means we need to resolve it for the functions frame of reference
				VALUE* func = interpret(tree->left, frame);
				CLOSURE* func_tree = (CLOSURE*)func->v.function;
				func_name = (TOKEN*)func_tree->code->left->right->left->left;
				return lexical_call_method(func_name, tree->right, func_tree->frame);
			}
		case STRING_LITERAL:;
			// Return token with string
			VALUE* string_lit = (VALUE*)malloc(sizeof(VALUE));
			string_lit->type = mmcSTRING;
			string_lit->v.string = ((TOKEN*)tree)->lexeme;
			return string_lit;
		case WHILE:;
			VALUE* while_ret = NULL;
			// Check for a return in the while loop.
			FRAME* new_scope_while = new_frame();
			new_scope_while->next = frame;
			while (interpret(tree->left, frame)->v.boolean == 1 && (while_ret == NULL || !while_ret->is_func_ret))
			{
				while_ret = interpret(tree->right, new_scope_while);
			}
			return while_ret;
		default:
		break;
  	}
}