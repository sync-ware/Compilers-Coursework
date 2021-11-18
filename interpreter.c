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