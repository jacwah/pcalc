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
#include "pcalc.h"

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
		if (stack->array == NULL) {
			free(stack);
			return NULL;
		}
		else {
			return stack;
		}
	}
}

void stack_free(struct stack *stack)
{
	free(stack->array);
	free(stack);
}

enum retcode stack_push(struct stack *stack, int value)
{
	if (stack->top == stack->size) {
		stack->size *= 2;
		stack->array = realloc(stack->array,
							   stack->size * sizeof(*stack->array));

		if (stack->array == NULL) {
			return R_MEMORY_ALLOC;
		}
	}
	stack->array[stack->top++] = value;
	return R_OK;
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

int stack_peek(struct stack *stack)
{
	assert(stack->top != 0);

	return stack->array[stack->top - 1];
}

int stack_is_empty(struct stack *stack)
{
	return stack->top == 0;
}

int stack_size(struct stack *stack)
{
	return stack->top;
}
