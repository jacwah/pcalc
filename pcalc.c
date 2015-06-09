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
#include <assert.h>
#include "pcalc.h"
#include "stack.h"

#define MAX_TOKENS 32

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
struct token **pn_parse(char *expr)
{
	// Will be NULL terminated if not full
	struct token **prog = calloc(MAX_TOKENS, sizeof(*prog));
	struct token *prog_reverse[MAX_TOKENS];
	int end = 0;
	char *expr_base = expr;

	while (isspace(*expr))
		expr++;

	while (*expr != '\0') {
		struct token *token = pn_read_token(&expr);

		if (token) {
			if (end < MAX_TOKENS) {
				prog_reverse[end++] = token;
			}
			else {
				fputs("Too many tokens\n", stderr);
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

	for (int i = 0; i < end; i++) {
		prog[i] = prog_reverse[end - i - 1];
	}

	return prog;
}

int pn_eval_binary_op(struct stack *v_stack, enum token_type type)
{
	if (stack_size(v_stack) >= 2) {
		int lval = stack_pop(v_stack);
		int rval = stack_pop(v_stack);
		int result = 0;

		switch (type) {
			case OP_ADD: result = lval + rval; break;
			case OP_SUB: result = lval - rval; break;
			case OP_MULT:result = lval * rval; break;
			case OP_DIV: result = lval / rval; break;
			default: assert(0);
		}
		stack_push(v_stack, result);
		return 1;
	}
	else {
		return 0;
	}
}

int pn_eval(struct token **program, int *result)
{
	struct stack *v_stack = stack_new(MAX_TOKENS);

	for (int i = 0; i < MAX_TOKENS && program[i] != NULL; i++) {
		struct token *token = program[i];

		switch (token->type) {
			case VALUE:
				stack_push(v_stack, token->value);
				break;

			case OP_ADD:
			case OP_SUB:
			case OP_MULT:
			case OP_DIV:
				if (!pn_eval_binary_op(v_stack, token->type)) {
					fputs("Too few value for binary operator\n", stderr);
					exit(1);
				}
				break;

			default:
				fprintf(stderr, "Uknown token type %d\n", token->type);

				return 0;
		}
	}

	if (stack_size(v_stack) == 1) {
		*result = stack_pop(v_stack);
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

	struct token **program = pn_parse(str);

	int result = 0;

	if (pn_eval(program, &result)) {
		printf("Expression value: %d\n", result);

		return 0;
	}
	else {
		return 1;
	}
}