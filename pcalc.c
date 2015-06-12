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

// Return NULL if a) memory alloc fails or b) eof was encountered before any
// characters were read
char *read_line(FILE *file)
{
	size_t size = 16;
	size_t len = 0;
	char *line = malloc(size);

	if (line == NULL)
		return NULL;

	for(;;) {
		int c;

		switch (c = fgetc(file)) {
			case EOF:
				if (len == 0) {
					free(line);
					return NULL;
				}
				else {
					line[len+1] = '\0';
					return line;
				}

			case '\n':
				line[len+1] = '\0';
				return line;

			default:
				line[len++] = c;
		}

		if (len + 1 >= size) {
			size *= 2;
			line = realloc(line, size);
			if (line == NULL)
				return NULL;
		}
	}
}

// Read the first token in expr. Token parameter must be allocated memory.
enum retcode pn_read_token(struct token *token, char *expr)
{
	char *head = expr;

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
		return R_OK;
	}
	else {
		return R_UKNOWN_TOKEN;
	}
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

// Parse and evaluate a string Polish Notation expression
enum retcode pn_eval_str(int *result, char *expr)
{
	struct stack *v_stack = stack_new(MAX_TOKENS);
	char *walk = expr + strlen(expr) - 1;

	// Start at end of expression and skip to beginning of last token
	while (walk > expr && isspace(*walk))
		walk--;
	while (walk > expr && !isspace(*(walk - 1)))
		walk--;

	while (walk >= expr) {
		struct token token;

		enum retcode ret = pn_read_token(&token, walk);

		if (ret == R_OK) {
			switch (token.type) {
				case VALUE:
					stack_push(v_stack, token.value);
					break;

				case OP_ADD:
				case OP_SUB:
				case OP_MULT:
				case OP_DIV:
					if (pn_eval_binary_op(v_stack, token.type) ==
						R_NOT_ENOUGH_VALUES) {
						return R_INVALID_EXPRESSION;
					}
					break;

				default:
					assert(0);
			}
		}
		else if (ret == R_UKNOWN_TOKEN){
			return R_UKNOWN_TOKEN;
		}

		walk--;
		while (walk > expr && isspace(*walk))
			walk--;
		while (walk > expr && !isspace(*(walk - 1)))
			walk--;

		if (walk == expr && isspace(*walk))
			break;
	}

	if (stack_size(v_stack) == 1) {
		*result = stack_pop(v_stack);
		return R_OK;
	}
	else {
		return R_INVALID_EXPRESSION;
	}

	return R_OK;
}

int prompt_loop()
{
	fprintf(stderr, "Type 'q' or 'quit' to exit\n");
	for (;;) {
		char *expr;

		printf("pcalc> ");
		expr = read_line(stdin);

		if (expr != NULL) {
			if (strcmp(expr, "q") == 0 || strcmp(expr, "quit") == 0) {
				return EXIT_SUCCESS;
			}
			else if (expr[0] == '\0') {
				continue;
			}
			else {
				int result;
				enum retcode ret = pn_eval_str(&result, expr);

				if (ret == R_OK) {
					printf("%d\n", result);
				}
				else if (ret == R_INVALID_EXPRESSION) {
					fprintf(stderr, "Invalid expression\n");
				}
				else if (ret == R_UKNOWN_TOKEN){
					fprintf(stderr, "Uknown token\n");
				}
				else {
					assert(0);
				}
			}
		}
		else {
			fprintf(stderr, "Reading input failed\n");
			return EXIT_FAILURE;
		}
	}
}

int main(int argc, char **argv)
{
	if (argc == 1) {
		return prompt_loop();
	}
	else {
		char str[1024];

		str[0] = '\0';
		for (int i = 1; i < argc; i++) {
			strcat(str, argv[i]);
			strcat(str, " ");
		}

		puts(str);

		int result = 0;
		enum retcode ret = pn_eval_str(&result, str);

		if (ret == R_OK) {
			printf("Expression value: %d\n", result);
			return EXIT_SUCCESS;
		}
		else if (ret == R_INVALID_EXPRESSION) {
			fputs("Invalid expression\n", stderr);
			return EXIT_FAILURE;
		}
		else if (ret == R_UKNOWN_TOKEN) {
			fputs("Uknown token\n", stderr);
			return EXIT_FAILURE;
		}
	}
}