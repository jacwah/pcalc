//
//  pcalc.c
//  
//
//  Copyright 2015 Jacob Wahlgren
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
// after the read token. Token parameter must be allocated memory.
enum retcode pn_read_token(struct token *token, char **expr)
{
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
				return R_UKNOWN_TOKEN;
			}
	}

	if (isspace(*head) || *head == '\0') {
		*expr = head;
		return R_OK;
	}
	else {
		return R_UKNOWN_TOKEN;
	}
}

// Parse a string Polish Notation expression
enum retcode pn_parse(struct token ***out, char *expr)
{
	// Will be NULL terminated if not full
	struct token **prog = calloc(MAX_TOKENS, sizeof(*prog));
	struct token *prog_reverse[MAX_TOKENS];
	int end = 0;

	if (prog == NULL) {
		return R_MEMORY_ALLOC;
	}

	while (isspace(*expr))
		expr++;

	while (*expr != '\0') {
		struct token *token = malloc(sizeof(token));

		if (token == NULL) {
			free(prog);
			return R_MEMORY_ALLOC;
		}

		enum retcode ret = pn_read_token(token, &expr);

		if (ret == R_OK) {
			if (end < MAX_TOKENS) {
				prog_reverse[end++] = token;
			}
			else {
				free(prog);
				return R_OUT_OF_BOUNDS;
			}
		}
		else if (ret == R_UKNOWN_TOKEN){
			return R_UKNOWN_TOKEN;
		}

		while (isspace(*expr))
			expr++;
	}

	for (int i = 0; i < end; i++) {
		prog[i] = prog_reverse[end - i - 1];
	}
	*out = prog;

	return R_OK;
}

enum retcode pn_eval_binary_op(struct stack *v_stack, enum token_type type)
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
		return R_OK;
	}
	else {
		return R_NOT_ENOUGH_VALUES;
	}
}

enum retcode pn_eval(struct token **program, int *result)
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
				if (pn_eval_binary_op(v_stack, token->type) ==
						R_NOT_ENOUGH_VALUES) {
					return R_INVALID_EXPRESSION;
				}
				break;

			default:
				assert(0);
		}
	}

	if (stack_size(v_stack) == 1) {
		*result = stack_pop(v_stack);
		return R_OK;
	}
	else {
		return R_INVALID_EXPRESSION;
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

	struct token **program;
	enum retcode ret_p = pn_parse(&program, str);

	if (ret_p == R_OK) {
		int result = 0;
		enum retcode ret_e = pn_eval(program, &result);

		if (ret_e == R_OK) {
			printf("Expression value: %d\n", result);

			return EXIT_SUCCESS;
		}
		else {
			fputs("Invalid expression\n", stderr);
			return EXIT_FAILURE;
		}
	}
	else {
		char *str = NULL;
		if (ret_p == R_OUT_OF_BOUNDS) {
			str = "Too many tokens\n";
		}
		else if (ret_p == R_UKNOWN_TOKEN) {
			str = "Uknown token in expression\n";
		}
		else if (ret_p == R_MEMORY_ALLOC) {
			str = "Memory allocation failed\n";
		}
		fputs(str, stderr);
		return EXIT_FAILURE;
	}
}