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
#include <errno.h>
#include "pcalc.h"
#include "stack.h"

#define MIN_STACK_SIZE 16

int is_undefined_add(int a, int b)
{
	return (a > 0 && b > INT_MAX - a) ||
		   (a < 0 && b < INT_MIN - a);
}

int is_undefined_sub(int a, int b)
{
	return (b > 0 && a < INT_MIN + b) ||
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
# if INT_MIN < -INT_MAX
	if (a == INT_MIN && b == -1)
		return 1;
#endif
	return b == 0;
}

// str is a null terminated string accepted by strtol
enum retcode parse_int(int *result, char *str)
{
	char *endp;
	long value;

	value = strtol(str, &endp, 0);

	if (errno == ERANGE || value > INT_MAX || value < INT_MIN) {
		return R_OUT_OF_BOUNDS;
	}
	else if (*endp != '\0') {
		return R_UKNOWN_TOKEN;
	}
	else {
		*result = (int)value;
		return R_OK;
	}
}

const char *retcode_str(enum retcode ret)
{
	switch(ret) {
		case R_OK:					return "No errors occurred";
		case R_MEMORY_ALLOC:		return "Memory allocation failed";
		case R_OUT_OF_BOUNDS:		return "Value out of bounds";
		case R_NOT_ENOUGH_VALUES:	return "Not enough values";
		case R_UKNOWN_TOKEN:		return "Uknown token";
		case R_INVALID_EXPRESSION:	return "Invalid expression";
		default: assert(0);
	}
}

// Read the token pointed to by expr. Token parameter must be allocated memory.
enum retcode pn_read_token(struct token *token, char *expr)
{
#define IS_DELIM(c) (isspace(c) || (c) == '\0')

	char *head = expr;

	if (head[0] == '+' && IS_DELIM(head[1])) {
		token->type = OP_ADD;
		head++;
	}
	else if (head[0] == '-' && IS_DELIM(head[1])) {
		token->type = OP_SUB;
		head++;
	}
	else if (head[0] == '*' && IS_DELIM(head[1])) {
		token->type = OP_MULT;
		head++;
	}
	else if (head[0] == '/' && IS_DELIM(head[1])) {
		token->type = OP_DIV;
		head++;
	}
	else if (isdigit(head[0]) || head[0] == '+' || head[0] == '-') {
		char buf[32];
		int result;
		enum retcode ret;

		// buf will always be zero terminated
		memset(buf, 0, sizeof(buf));

		for (int i = 0; i + 1 < sizeof(buf) && !IS_DELIM(*head); i++) {
			buf[i] = *head++;
		}

		ret = parse_int(&result, buf);

		if (ret == R_OUT_OF_BOUNDS)
			return R_OUT_OF_BOUNDS;
		else if (ret == R_UKNOWN_TOKEN)
			return R_UKNOWN_TOKEN;
		else
			token->value = result;

		token->type = VALUE;
	}
	else {
		return R_UKNOWN_TOKEN;
	}

	if (IS_DELIM(*head))
		return R_OK;
	else if (token->type == VALUE && isdigit(*head))
		return R_OUT_OF_BOUNDS;
	else
		return R_UKNOWN_TOKEN;

#undef IS_DELIM
}

enum retcode pn_eval_binary_op(struct stack *v_stack, enum token_type type)
{
	if (stack_size(v_stack) >= 2) {
		int lval = stack_pop(v_stack);
		int rval = stack_pop(v_stack);
		int result = 0;

		switch (type) {
			case OP_ADD:
				if (is_undefined_add(lval, rval))
					return R_OUT_OF_BOUNDS;
				else
					result = lval + rval;
				break;

			case OP_SUB:
				if (is_undefined_sub(lval, rval))
					return R_OUT_OF_BOUNDS;
				else
					result = lval - rval;
				break;

			case OP_MULT:
				if (is_undefined_mult(lval, rval))
					return R_OUT_OF_BOUNDS;
				else
					result = lval * rval;
				break;

			case OP_DIV:
				if (is_undefined_div(lval, rval))
					return R_OUT_OF_BOUNDS;
				else
					result = lval / rval;
				break;

			default:
				assert(0);
		}

		if (stack_push(v_stack, result) == R_MEMORY_ALLOC)
			return R_MEMORY_ALLOC;
		else
			return R_OK;
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

					if (ret == R_NOT_ENOUGH_VALUES)
						return R_INVALID_EXPRESSION;
					else if (ret == R_MEMORY_ALLOC)
						return R_MEMORY_ALLOC;
					else if (ret == R_OUT_OF_BOUNDS)
						return R_OUT_OF_BOUNDS;
					else if (ret != R_OK)
						assert(0);
				}
					break;

				default:
					assert(0);
			}
		}
		else if (ret == R_UKNOWN_TOKEN){
			return R_UKNOWN_TOKEN;
		}
		else if (ret == R_OUT_OF_BOUNDS) {
			return R_OUT_OF_BOUNDS;
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
				else {
					fprintf(stderr, "%s\n", retcode_str(ret));
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
		else {
			fprintf(stderr, "%s\n", retcode_str(ret));
			return EXIT_FAILURE;
		}
	}
}
