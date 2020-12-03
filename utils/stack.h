/**************************************************************
* Class:  CSC-415
* Name: Mark Jovero
* Student ID:   916691664
* Project: Basic File System
*
* File: stack.c
*
* Description: A stack implementation
*
**************************************************************/

typedef struct {
    int capacity;
    int current_size;
    int * content;
} fs_stack;


fs_stack * stack_create(int size);
fs_stack * stack_copy(fs_stack * src);

int stack_push(int content, fs_stack * stack);
int stack_pop(fs_stack * stack);
int stack_peek(fs_stack * stack);
int stack_size(fs_stack * stack);
