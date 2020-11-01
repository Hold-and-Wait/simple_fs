

struct stack_util create_stack();
void test_test();

struct stack_util create_stack(int size);

void expand_stack(int size, struct stack_util * stack);
void stack_push(int content, struct stack_util * stack);
int stack_pop(struct stack_util * stack);
int stack_peek(struct stack_util * stack);
int stack_size(struct stack_util * stack);

struct stack_util {
    int capacity;
    int current_size;
    struct stack_contents * contents;
};

struct stack_contents {
    int data;
};