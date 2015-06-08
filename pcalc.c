//
//  pcalc.c
//  
//
//  Created by Jacob Wahlgren on 2015-06-08.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define STACK_SIZE 8

// Only positive exponents
int ipow(int base, int exp)
{
	int result = 1;
	while (exp)
	{
		if (exp & 1)
			result *= base;
		exp >>= 1;
		base *= base;
	}

	return result;
}

enum token_type {
	NONE,
	VALUE,
	OP_ADD,
	OP_SUB,
};

// value will only be defined if type is VALUE
struct token {
	enum token_type type;
	int value;
};

struct stack {
	struct token **array;
	size_t size;
	size_t top;
};

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

// str is a null terminated string of only digits
// does not handle overflow!!!
int parse_int(char *str)
{
	int value = 0;

	for (int exp = strlen(str) - 1; *str != '\0'; str++, exp--)
	{
		value += ipow(10, exp) * (*str - '0');
	}

	return value;
}

// Read the first token in expr and advance it to point at the first character
// after the read token
struct token *pn_read_token(char **expr)
{
	struct token *token = malloc(sizeof(struct token));

	if (token == NULL)
		return NULL;
	else {
		char *head = *expr;

		switch (*head) {
			case '+': token->type = OP_ADD; head++; break;
			case '-': token->type = OP_SUB; head++; break;
			default:
				if (isdigit(*head)) {
					const int buf_size = 16;
					char buf[buf_size];

					// buf will always be zero terminated
					memset(buf, 0, sizeof(buf));

					for (int i = 0; i + 1 < buf_size && isdigit(*head); i++) {
						buf[i] = *head++;
					}

					token->type = VALUE;
					token->value = parse_int(buf);
				}
				else {
					token->type = NONE;
				}
		}

		*expr = head;

		if ((isspace(*head) || *head == '\0') && token->type != NONE) {
			return token;
		}
		else {
			free(token);
			return NULL;
		}
	}
}

// Parse a string Polish Notation expression
struct stack *pn_parse(char *expr)
{
	struct stack *stack = stack_new(STACK_SIZE);
	char *expr_base = expr;

	while (isspace(*expr))
		expr++;

	while (*expr != '\0') {
		struct token *token = pn_read_token(&expr);

		if (token) {
			if (!stack_push(stack, token)) {
				fputs("Stack size too small\n", stderr);
				exit(1);
			}
		}
		else {
			fprintf(stderr, "Invalid token at index %ld\n", expr - expr_base);
			exit(1);
		}

		while (isspace(*expr))
			expr++;
	}

	return stack;
}

int main(int argc, char **argv)
{
	char str[1024];

	str[0] = '\0';
	for (int i = 1; i < argc; i++) {
		strcat(str, argv[i]);
		strcat(str, " ");
	}

	puts(str);

	struct stack *stack = pn_parse(str);

	if (stack) {
		struct token *token = NULL;
		puts("Stack contents:");

		while (NULL != (token = stack_pop(stack))) {
			printf("Type: %d, value %d\n", token->type, token->value);
			free(token);
		}

		return 0;
	}
	else {
		fputs("NULL stack\n", stderr);

		return 1;
	}
}