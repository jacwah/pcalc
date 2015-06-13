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
#include <limits.h>
#include "pcalc.h"
#include "stack.h"

#define MIN_STACK_SIZE 16

int is_undefined_add(int a, int b)
{
	return (a > 0 && b > INT_MAX - a) ||
		   (a < 0 && b < INT_MAX - a);
}

int is_undefined_sub(int a, int b)
{
	return (b > 0 && a < INT_MAX + b) ||
		   (b < 0 && a > INT_MAX + b);
}

int is_undefined_mult(int a, int b)
{
	if (a > 0) {
		if (b > 0) {
			if (a > INT_MAX / b) {
				return 1;
			}
		}
		else {
			if (b < INT_MIN / a) {
				return 1;
			}
		}
	}
	else {
		if (b > 0) {
			if (a < INT_MIN / b) {
				return 1;
			}
		}
		else {
			if (a != 0 && b < INT_MAX / a) {
				return 1;
			}
		}
	}

	return 0;
}

int is_undefined_div(int a, int b)
{
	return b == 0 || (a == INT_MIN && b == -1);
}

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
			case OP_ADD:
				if (is_undefined_add(lval, rval)) {
					return R_OUT_OF_BOUNDS;
				}
				else {
					result = lval + rval;
				}
				break;

			case OP_SUB:
				if (is_undefined_sub(lval, rval)) {
					return R_OUT_OF_BOUNDS;
				}
				else {
					result = lval - rval;
				}
				break;

			case OP_MULT:
				if (is_undefined_mult(lval, rval)) {
					return R_OUT_OF_BOUNDS;
				}
				else {
					result = lval * rval;
				}
				break;

			case OP_DIV:
				if (is_undefined_div(lval, rval)) {
					return R_OUT_OF_BOUNDS;
				}
				else {
					result = lval / rval;
				}
				break;

			default: assert(0);
		}

		if (stack_push(v_stack, result) == R_MEMORY_ALLOC) {
			return R_MEMORY_ALLOC;
		}
		else {
			return R_OK;
		}
	}
	else {
		return R_NOT_ENOUGH_VALUES;
	}
}

// Parse and evaluate a string Polish Notation expression
enum retcode pn_eval_str(int *result, char *expr)
{
	struct stack *v_stack = stack_new(MIN_STACK_SIZE);
	char *walk = expr + strlen(expr) - 1;

	if (v_stack == NULL) {
		return R_MEMORY_ALLOC;
	}

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
					if (stack_push(v_stack, token.value) == R_MEMORY_ALLOC) {
						stack_free(v_stack);
						return R_MEMORY_ALLOC;
					}
					break;

				case OP_ADD:
				case OP_SUB:
				case OP_MULT:
				case OP_DIV:
				{
					enum retcode ret = pn_eval_binary_op(v_stack, token.type);
					if (ret == R_NOT_ENOUGH_VALUES) {
						return R_INVALID_EXPRESSION;
					}
					else if (ret == R_MEMORY_ALLOC) {
						return R_MEMORY_ALLOC;
					}
					else if (ret == R_OUT_OF_BOUNDS) {
						return R_OUT_OF_BOUNDS;
					}
					else if (ret != R_OK){
						assert(0);
					}
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
		size_t len;

		printf("pcalc> ");

		if (getline(&expr, &len, stdin) > 0) {
			if (expr[0] == '\n') {
				continue;
			}
			else if (strcmp(expr, "q\n") == 0 || strcmp(expr, "quit\n") == 0) {
				return EXIT_SUCCESS;
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
				else if (ret == R_UKNOWN_TOKEN) {
					fprintf(stderr, "Uknown token\n");
				}
				else if (ret == R_MEMORY_ALLOC) {
					fprintf(stderr, "Memory allocation failed\n");
				}
				else if (ret == R_OUT_OF_BOUNDS) {
					fprintf(stderr, "Result out of bounds\n");
				}
				else {
					assert(0);
				}
			}
		}
		else {
			perror("Reading input failed");
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
		else if (ret == R_MEMORY_ALLOC) {
			fputs("Memory allocation failed\n", stderr);
			return EXIT_FAILURE;
		}
		else if (ret == R_OUT_OF_BOUNDS) {
			fprintf(stderr, "Result out of bounds\n");
			return EXIT_FAILURE;
		}
	}
}