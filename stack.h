//
//  stack.h
//  
//
//  Created by Jacob Wahlgren on 2015-06-09.
//
//

#ifndef STACK_H
#define STACK_H

// Need struct token
#include "pcalc.h"

struct stack {
	struct token **array;
	size_t size;
	size_t top;
};

void stack_init(struct stack *stack, size_t size);
struct stack *stack_new(size_t size);
void stack_free(struct stack *stack);
int stack_push(struct stack *stack, struct token *value);
struct token *stack_pop(struct stack *stack);
int stack_is_empty(struct stack *stack);

#endif
