#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

STACK* new_stack(){
	STACK* stack = (STACK*)malloc(sizeof(STACK));
	stack->size = 0;
	stack->top = -1;
	return stack;
}

void** copy_array(void** array, int a_size){
	void** copy = (void**)malloc(sizeof(void*)*(a_size*2));
	for (int i = 0; i < a_size; i++){
		copy[i] = array[i];
	}
	if (a_size > 0){
		free(array);
	}
	return copy;
}

void push(STACK* stack, void* object){
	stack->top++;
	if (stack->top == stack->size){
		void** copy = copy_array(stack->contents, stack->size);
		copy[stack->top] = object;
		stack->contents = copy;
		stack->size++;
	} else {
		stack->contents[stack->top] = object;
	}
	
}

void* pop(STACK* stack){
	void* top = stack->contents[stack->top];
	stack->contents[stack->top--] = NULL;
	return top;
}

void print_stack(STACK* stack){
	printf("Size: %d, Top: %p\n", stack->size, stack->contents[stack->top]);
	printf("Contents:\n");
	for(int i = stack->size-1; i > -1; i--){
		printf("%d: %p\n", i, stack->contents[i]);
	}
}

void free_stack(STACK* stack){
	free(stack->contents);
	free(stack);
}