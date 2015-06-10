//
//  stack.c
//  
//
//  Copyright 2015 Jacob Wahlgren
//
//

#include <stdlib.h>
#include <assert.h>
#include "stack.h"

void stack_init(struct stack *stack, size_t size)
{
	stack->array = calloc(size, sizeof(*stack->array));
	stack->size = size;
	stack->top = 0;
}

struct stack *stack_new(size_t size)
{
	struct stack *stack = malloc(sizeof(struct stack));
	if (stack == NULL) {
		return NULL;
	}
	else {
		stack_init(stack, size);
		return stack;
	}
}

void stack_free(struct stack *stack)
{
	free(stack->array);
	free(stack);
}

int stack_push(struct stack *stack, int value)
{
	if (stack->top < stack->size) {
		stack->array[stack->top++] = value;
		return 1;
	}
	else {
		return 0;
	}
}

// Can only call if stack is not empty
int stack_pop(struct stack *stack)
{
	assert(stack->top != 0);

	int value = stack->array[stack->top - 1];
	stack->array[stack->top - 1] = 0;
	stack->top--;
	return value;
}

int stack_is_empty(struct stack *stack)
{
	return stack->top == 0;
}

int stack_size(struct stack *stack)
{
	return stack->top;
}
