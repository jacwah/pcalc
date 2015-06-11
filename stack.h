//
//  stack.h
//  
//
//  Copyright 2015 Jacob Wahlgren
//
//

#ifndef STACK_H
#define STACK_H

struct stack {
	int *array;
	size_t size;
	size_t top;
};

void stack_init(struct stack *stack, size_t size);
struct stack *stack_new(size_t size);
void stack_free(struct stack *stack);
void stack_push(struct stack *stack, int value);
int stack_pop(struct stack *stack);
int stack_is_empty(struct stack *stack);
int stack_is_full(struct stack *stack);
int stack_size(struct stack *stack);

#endif
