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

#include <stdio.h>
#include "stack.h"
#include <stdlib.h>

void test_test() {
    printf("!@#!@#!#\n");
}

struct stack_util * temp_stack;

struct stack_util create_stack(int size) {
    struct stack_util * new_stack = malloc(size * sizeof(struct stack_util));
    new_stack[0].capacity = size;
    new_stack->contents = malloc(size * sizeof(struct stack_contents));
    temp_stack = new_stack;
    return  * new_stack;
}

void expand_stack(int size, struct stack_util * stack) {
    stack = realloc(&stack, (stack->current_size + size) * sizeof(struct stack_util));
    stack->capacity = stack->current_size + size;
    stack->contents = realloc(stack->contents, (stack->current_size + size) * sizeof(struct stack_contents));
}

void stack_push(int content, struct stack_util * stack) {
    if (stack->current_size == stack->capacity) {
        expand_stack(10, stack);
    }

    stack->contents[stack->current_size * sizeof(struct stack_contents)].data = content;
    stack->current_size++;
}

struct stack_util stack_copy(struct stack_util * dest, struct stack_util * src) {
    *dest = create_stack(src->capacity);

    for (int i = 0; i < stack_size(src) * sizeof(struct stack_contents); i += sizeof(struct stack_contents)) {
    }

    return * dest;
}

int stack_peek(struct stack_util * stack) {
    return stack->contents[(stack->current_size-1) * sizeof(struct stack_contents)].data;
}

int stack_pop(struct stack_util * stack) {
    stack->current_size--;
    int temp = stack->contents[stack->current_size * sizeof(struct stack_contents)].data;
    stack->contents[stack->current_size * sizeof(struct stack_contents)].data = 0;
    return temp;
}

int stack_size(struct stack_util * stack) {
    return stack->current_size;
}