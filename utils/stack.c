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
#include <stdlib.h>
#include <string.h>
#include "stack.h"

struct stack_util * stack_create(int size) {
	// Allocate new stack
	struct stack_util * new_stack = malloc(sizeof(struct stack_util));
	
	// Allocate content
	new_stack->content = malloc(sizeof(int) * size);
	new_stack->capacity = size;
	
	printf("Creating a new stack (%d).\n", size);
	return new_stack;
}

int stack_push(int value, struct stack_util * stack) {
	int * iter_ptr = stack->content;
	
	if (stack->current_size < stack->capacity) {
		for (int i = 0; i < stack->current_size; i++, iter_ptr++);
		*iter_ptr = value;
		printf("PUSH %d\n", * iter_ptr);
		stack->current_size++;
		return value;
	} else {
		// expand
		stack->capacity = stack->capacity * 2;
		stack->content = realloc(stack->content, sizeof(int) * stack->capacity);
		stack_push(value, stack);
	}
	
	return -1;
}

int stack_peek(struct stack_util * stack) {
	int * iter_ptr = stack->content;
	
	for (int i = 0; i < stack->current_size-1; i++, iter_ptr++);
	
	return * iter_ptr;
}




























