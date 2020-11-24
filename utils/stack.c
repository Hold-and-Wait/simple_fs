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

fs_stack * stack_create(int size) {
	// Allocate new stack
	fs_stack * new_stack = malloc(sizeof(fs_stack ));
	
	// Allocate content
	new_stack->content = malloc(sizeof(int) * size);
	new_stack->capacity = size;
	
	return new_stack;
}

fs_stack * stack_copy(fs_stack * src) {
	fs_stack * stack_cpy = stack_create(stack_size(src));
	
	stack_cpy->current_size = src->current_size;	
	// copy contents
	
	int * src_content_ptr = src->content;
	int * cpy_content_ptr = stack_cpy->content;
	
	//memcpy(stack_cpy->content, src->content, stack_size(src) * sizeof(int));
	for (int i = 0; i < stack_size(src); i++, src_content_ptr++, cpy_content_ptr++) {
		*cpy_content_ptr = *src_content_ptr;
	}
	
	return stack_cpy;
}

int stack_push(int value, fs_stack * stack) {
	int * iter_ptr = stack->content;
	
	if (stack->current_size < stack->capacity) {
		for (int i = 0; i < stack->current_size; i++, iter_ptr++);
		*iter_ptr = value;
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

int stack_peek(fs_stack * stack) {
	if (stack->current_size == 0)
		return -1;
	int * iter_ptr = stack->content;
	for (int i = 0; i < stack->current_size-1; i++, iter_ptr++);
	return * iter_ptr;
}

int stack_pop(fs_stack * stack) {
	if (stack->current_size == 0)
		return -1;

	int * iter_ptr = stack->content;
	for (int i = 0; i < stack->current_size-1; i++, iter_ptr++);
	int val_to_remove = * iter_ptr;
	*iter_ptr = 0;
	iter_ptr = NULL;
	stack->current_size--;
	
	return val_to_remove;
}

int stack_size(fs_stack * stack) {
	return stack->current_size;
}
























