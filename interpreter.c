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

BINDING* new_binding(NODE* name, VALUE* val, BINDING* next){
	BINDING* binding = (BINDING*)malloc(sizeof(BINDING));
	binding->name = (TOKEN*)name;
	binding->val = val;
	binding->next = next;
	return binding;
}

BINDING* gen_bindings(NODE* ids, NODE* args, FRAME* frame, BINDING* bindings){
	printf("Type binding: %d\n", ids->left->type);
	if (ids->left->type != VOID){
		if (ids->type != 44){
			printf("Id type: %d\n", ids->right->left->type);
			bindings = new_binding(ids->right->left, interpret(args->left, frame), bindings);
		} else {
			printf("Comma found\n");
			bindings = gen_bindings(ids->left, args->left, frame, bindings);
			bindings = gen_bindings(ids->right, args->right, frame, bindings);
		}
		return bindings;
	} else {
		return NULL;
	}
}

FRAME* extend_frame(FRAME* env, NODE* ids, NODE* args){
	FRAME* new_env = new_frame();
	
	BINDING* bindings = NULL;
	// { // Limit scope
	// 	NODE* ip;
	// 	NODE* ap;
	// 	for (ip = ids, ap = args; (ip != NULL) && (ap != NULL); ip = ip->right, ap = ap->right){
	// 		if (ip->type == 44){

	// 		} else {
	// 			bindings = new_binding(ip->right->left, interpret(ap->left, env), bindings);
	// 		}
	// 	}
	// }

	//new_env->binding = bindings;
	printf("Extending bindings\n");
	new_env->binding = gen_bindings(ids, args, env, bindings);
	printf("Bidnings finished\n");
	return new_env;
}

VALUE* lexical_call_method(TOKEN* name, NODE* args, FRAME* frame){
	printf("Calling: %s\n", name->lexeme);
	CLOSURE* f = generate_closure(name, frame);
	FRAME* new_env = extend_frame(frame, f->args, args);
	new_env->next = f->frame;
	printf("Body type: %d\n", f->body->type);
	return interpret(f->body, new_env);
}

VALUE* add_values(VALUE* left_operand, VALUE* right_operand){
  left_operand->v.integer = left_operand->v.integer + right_operand->v.integer;
  free(right_operand);
  return left_operand;
}

VALUE* sub_values(VALUE* left_operand, VALUE* right_operand){
  left_operand->v.integer = left_operand->v.integer - right_operand->v.integer;
  free(right_operand);
  return left_operand;
}

VALUE* div_values(VALUE* left_operand, VALUE* right_operand){
  left_operand->v.integer = left_operand->v.integer / right_operand->v.integer;
  free(right_operand);
  return left_operand;
}

VALUE* mul_values(VALUE* left_operand, VALUE* right_operand){
  left_operand->v.integer = left_operand->v.integer * right_operand->v.integer;
  free(right_operand);
  return left_operand;
}

VALUE* mod_values(VALUE* left_operand, VALUE* right_operand){
  left_operand->v.integer = left_operand->v.integer % right_operand->v.integer;
  free(right_operand);
  return left_operand;
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

VALUE* declare_function(NODE* func, FRAME* frame){
	BINDING* bindings = frame->binding;
	BINDING* new = (BINDING*)malloc(sizeof(BINDING));
	if (new != 0){
		VALUE* value = (VALUE*)malloc(sizeof(VALUE));
		value->type = mmcFUNC;
		value->v.function = (void*)func;
		TOKEN* token = new_token(mmcINT);
		token->lexeme = interpret(func->left->right->left, frame)->v.string;
		new->name = token;
		new->val = value;
		new->next = bindings;
		frame->binding = new;
		return new_value(mmcSTRING, (void*)&token->lexeme);
	}
}

CLOSURE* generate_closure(TOKEN* name, FRAME* frame){
	VALUE* val = get_variable(name, frame);
	CLOSURE* closure = (CLOSURE*)malloc(sizeof(CLOSURE));
	NODE* func_def = (NODE*)val->v.function;

	closure->code = func_def;
	closure->frame = frame;
	closure->body = func_def->right;
	closure->args = func_def->left->right->right;
	//printf("Argument type: %d\n", func_def->left->right->right->type);
	
	return closure;
}

VALUE* get_variable(TOKEN* var, FRAME* frame){
	printf("Looking up variable: %s\n", var->lexeme);
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
	printf("Variable not found\n");
	return NULL;
}

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

VALUE* equality_calculator(int type, NODE* tree, FRAME* frame){
	VALUE* left_operand = interpret(tree->left, frame);
	VALUE* right_operand = interpret(tree->right, frame);
	int equality;
	if (left_operand->type == mmcINT && right_operand->type == mmcINT){
		printf("Both ints\n");
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

VALUE* interpret(NODE *tree, FRAME* frame)
{
  	switch(tree->type){
    	case 68: //D
      		printf("Func def found.\n");
			return declare_function(tree, frame);

    	case RETURN:
      		printf("Return found.\n");
			VALUE* ret = interpret(tree->left, frame);
			ret->is_func_ret = 1;
			return ret;
		case LEAF:
			printf("Leaf found.\n");
			return interpret(tree->left, frame);
		case 43: //+
			printf("Plus found.\n");
			return add_values(interpret(tree->left, frame), interpret(tree->right, frame));
		case 45: //-
			printf("Minus found.\n");
			return sub_values(interpret(tree->left, frame), interpret(tree->right, frame));
		case 47: //(/)
			printf("Divide found.\n");
			return div_values(interpret(tree->left, frame), interpret(tree->right, frame));
		case 42: //(*)
			printf("Multiplication found.\n");
			return mul_values(interpret(tree->left, frame), interpret(tree->right, frame));
		case 37: //%
			printf("Modulo found.\n");
			return mod_values(interpret(tree->left, frame), interpret(tree->right, frame));
		case CONSTANT:;
			TOKEN *t = (TOKEN *)tree;
			printf("Constant found: %d.\n",t->value);
			return new_value(mmcINT, (void*)&t->value);
		case 59: // ;
			printf("Sequence found\n");
			VALUE* left_seq = interpret(tree->left, frame); // Go through and interpret the first part of the sequence
			//printf("Func type: %d\n", left_seq->is_func_ret);
			if (left_seq != NULL && left_seq->is_func_ret == 1){ // If left sequnce has a return value, we need to return this
				printf("left_seq: %d\n", left_seq->v.integer);
				return left_seq;
			}
			VALUE* right_seq = interpret(tree->right, frame);
			return right_seq;
		case ASSIGNMENT: // ~
			printf("Assignment found\n");
			if (tree->left->type == ASSIGNMENT || tree->left->type == 68){
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
				printf("Make new variable\n");
				return declare_variable(token, frame);
			}
		case INT:
			printf("Int type found.\n");
			int type = mmcINT;
			return new_value(mmcINT, (void*)&type);
		case IDENTIFIER:
			printf("Identifier found\n");
			
			TOKEN* id = (TOKEN*)tree;
			printf("Id: %s\n", id->lexeme);
			VALUE* found_id = get_variable(id, frame); // Check to see if it is already defined.
			if (found_id == NULL){
				return new_value(mmcSTRING, (void*)&id->lexeme);
			} else {
				printf("Variable found\n");
				return found_id;
			}
		case 61: // =
			printf("Equals found\n");
			VALUE* val = get_variable((TOKEN*)tree->left->left, frame);
			val->v.integer = interpret(tree->right, frame)->v.integer;
			return val;
		case IF:
			printf("If found\n");
			VALUE* condition = interpret(tree->left, frame);
			FRAME* new_scope = new_frame();
			new_scope->next = frame;
			//printf("type: %d\n", condition->type);
			if (condition->v.boolean){
				printf("Condition true\n");
				if (tree->right->type == ELSE){
					// If body is in this part of the tree if ELSE exists
					return interpret(tree->right->left, new_scope);
				} else {
					// If body here if no ELSE exists
					return interpret(tree->right, new_scope);
				}
			} else {
				printf("Condition false\n");
				if (tree->right->type == ELSE){
					return interpret(tree->right, new_scope);
				} else {
					return NULL;
				}
			}
		case ELSE:
			printf("Else found\n");

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
		case APPLY:
			printf("Function found\n");
			TOKEN* name = (TOKEN*)tree->left->left;
			printf("Function variable returned: %s\n", name->lexeme);
			return lexical_call_method(name, tree->right, frame);
		case 44:
			printf("Multiple arguments found\n");

		default:
		break;
  	}
}