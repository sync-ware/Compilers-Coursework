typedef struct stack{
    int top;
    int size;
    void** contents;
} STACK;

STACK* new_stack();
void push(STACK* stack, void* object);
void* pop(STACK* stack);
void print_stack(STACK* stack);
void free_stack(STACK* stack);