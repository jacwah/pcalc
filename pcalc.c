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
#include "pcalc.h"
#include "stack.h"

#define STACK_SIZE 32

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

struct token *token_new(enum token_type type, int value)
{
	struct token *token = malloc(sizeof(*token));

	if (token == NULL) {
		return NULL;
	}
	else {
		token->type = type;
		token->value = value;
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
			case '*': token->type = OP_MULT; head++; break;
			case '/': token->type = OP_DIV; head++; break;
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


int pn_stack_eval(struct stack *t_stack, int *result)
{
	struct stack *v_stack = stack_new(STACK_SIZE);

	while (!stack_is_empty(t_stack)) {
		struct token *token = stack_pop(t_stack);

		switch (token->type) {
			case VALUE:
				stack_push(v_stack, token);
				break;

			case OP_ADD:
			{
				struct token *lval = stack_pop(v_stack);
				struct token *rval = stack_pop(v_stack);

				if (rval && lval) {
					int result = lval->value + rval->value;
					struct token *newtoken = token_new(VALUE, result);
					stack_push(v_stack, newtoken);
				}
				else {
					fputs("Too few values for add\n", stderr);

					return 0;
				}
				break;
			}

			case OP_SUB:
			{
				struct token *lval = stack_pop(v_stack);
				struct token *rval = stack_pop(v_stack);

				if (rval && lval) {
					int result = lval->value - rval->value;
					struct token *newtoken = token_new(VALUE, result);
					stack_push(v_stack, newtoken);
				}
				else {
					fputs("No value to subtract\n", stderr);

					return 0;
				}
				break;
			}

			case OP_MULT:
			{
				struct token *lval = stack_pop(v_stack);
				struct token *rval = stack_pop(v_stack);

				if (rval && lval) {
					int result = lval->value * rval->value;
					struct token *newtoken = token_new(VALUE, result);
					stack_push(v_stack, newtoken);
				}
				else {
					fputs("Too few values to multiply\n", stderr);

					return 0;
				}
				break;
			}

			case OP_DIV:
			{
				struct token *lval = stack_pop(v_stack);
				struct token *rval = stack_pop(v_stack);

				if (rval && lval) {
					int result = lval->value / rval->value;
					struct token *newtoken = token_new(VALUE, result);
					stack_push(v_stack, newtoken);
				}
				else {
					fputs("Too few values to divide\n", stderr);

					return 0;
				}
				break;
			}

			default:
				fprintf(stderr, "Uknown token type %d\n", token->type);

				return 0;
		}
	}

	if (v_stack->top == 1) {
		struct token *token = stack_pop(v_stack);
		*result = token->value;
		free(token);
		return 1;
	}
	else {
		fputs("Invalid expression\n", stderr);
		exit(1);
	}
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

	int result = 0;

	if (pn_stack_eval(stack, &result)) {
		printf("Expression value: %d\n", result);

		return 0;
	}
	else {
		return 1;
	}
}