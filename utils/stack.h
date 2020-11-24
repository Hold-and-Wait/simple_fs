

struct stack_util create_stack();
void test_test();

struct stack_util * stack_create(int size);
struct stack_util stack_copy(struct stack_util * dest, struct stack_util * src);


void expand_stack(int size, struct stack_util * stack);
int stack_push(int content, struct stack_util * stack);
void rm_stack();
int empty_stack(struct stack_util * stack);
int stack_pop(struct stack_util * stack);
int stack_peek(struct stack_util * stack);
int stack_size(struct stack_util * stack);

struct stack_util {
    int capacity;
    int current_size;
    int * content;
};

struct stack_contents {
    int data;
};
