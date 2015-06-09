//
//  stack.c
//  
//
//  Created by Jacob Wahlgren on 2015-06-09.
//
//

#include <stdlib.h>
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
	for (int i = 0; i < stack->top; i++) {
		free(stack->array[i]);
	}
	free(stack->array);
	free(stack);
}

int stack_push(struct stack *stack, struct token *value)
{
	if (stack->top < stack->size) {
		stack->array[stack->top++] = value;
		return 1;
	}
	else {
		return 0;
	}
}

struct token *stack_pop(struct stack *stack)
{
	if (stack->top == 0) {
		return NULL;
	}
	else {
		struct token *token = stack->array[stack->top - 1];
		stack->array[stack->top - 1] = NULL;
		stack->top--;
		return token;
	}
}

int stack_is_empty(struct stack *stack)
{
	return stack->top == 0;
}
