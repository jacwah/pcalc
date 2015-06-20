//
//  pcalc.c
//  
//
//  Copyright 2015 Jacob Wahlgren
//
//

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>

#include "pcalc.h"
#include "stack.h"
#include "d_array.h"

#define MIN_STACK_SIZE 16

enum token_type {
	NONE,
	VALUE,
	OP_ADD,
	OP_SUB,
	OP_MULT,
	OP_DIV
};

// value will only be defined if type is VALUE
struct token {
	enum token_type type;
	int value;
};

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

int op_cmp(enum token_type op1, enum token_type op2)
{
	switch (op1) {
		case OP_ADD:
		case OP_SUB:
			if (op2 == OP_ADD || op2 == OP_SUB)
				return 0;
			else
				return -1;

		case OP_MULT:
		case OP_DIV:
			if (op2 == OP_ADD || op2 == OP_SUB)
				return 1;
			else
				return 0;

		default:
			assert(0);
	}
}

// Read the token pointed to by expr. Token parameter must be allocated memory.
enum retcode read_token(struct token *token, char *expr,
						char **endp, int *last_ans)
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
	else if (strncmp(head, "ans", strlen("ans")) == 0) {
		if (last_ans) {
			token->value = *last_ans;
			token->type = VALUE;
			head += strlen("ans");
		}
		else {
			return R_NO_LAST_ANS;
		}
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

	if (endp)
		*endp = head;

	if (IS_DELIM(*head))
		return R_OK;
	else if (token->type == VALUE && isdigit(*head))
		return R_OUT_OF_BOUNDS;
	else
		return R_UKNOWN_TOKEN;

#undef IS_DELIM
}

enum retcode pn_eval_binary_op(struct stack *v_stack, enum token_type type,
							   int is_reversed)
{
	if (stack_size(v_stack) >= 2) {
		int lval = stack_pop(v_stack);
		int rval = stack_pop(v_stack);
		int result = 0;

		if (is_reversed) {
			int cpy = rval;
			rval = lval;
			lval = cpy;
		}

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
// If an error occurs, *errp will point to the offending part of exrp
enum retcode pn_eval_str(int *result, char **errp, char *expr,
						 int is_reversed, int *last_ans)
{
	struct stack *v_stack = stack_new(MIN_STACK_SIZE);

	if (v_stack == NULL) {
		return R_MEMORY_ALLOC;
	}

	if (is_reversed) {
		*errp = expr;
		while (isspace(**errp))
			*errp += 1;
	}
	else {
		*errp = expr + strlen(expr) - 1;
		// Skip to beginning of last token
		while (*errp > expr && isspace(**errp))
			*errp -= 1;
		while (*errp > expr && !isspace((*errp)[-1]))
			*errp -= 1;
	}

	while (!is_reversed && *errp >= expr
		||  is_reversed && **errp != '\0') {
		struct token token;
		enum retcode ret;

		if (is_reversed)
			ret = read_token(&token, *errp, errp, last_ans);
		else
			ret = read_token(&token, *errp, NULL, last_ans);

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
					enum retcode ret = pn_eval_binary_op(v_stack, token.type,
														 is_reversed);

					if (ret != R_OK) {
						stack_free(v_stack);
						return ret;
					}
				}
					break;

				default:
					assert(0);
			}
		}
		else {
			stack_free(v_stack);

			switch (ret) {
				case R_UKNOWN_TOKEN:	return R_UKNOWN_TOKEN;
				case R_OUT_OF_BOUNDS:	return R_OUT_OF_BOUNDS;
				case R_NO_LAST_ANS:		return R_NO_LAST_ANS;

				default:   assert(0);
			}
		}

		if (is_reversed) {
			*errp += 1;
			while (isspace(**errp))
				*errp += 1;
		}
		else {
			*errp -= 1;
			while (*errp > expr && isspace(**errp))
				*errp -= 1;
			while (*errp > expr && !isspace((*errp)[-1]))
				*errp -= 1;

			if (*errp == expr && isspace(**errp))
				break;
		}
	}

	if (stack_size(v_stack) == 1) {
		*result = stack_pop(v_stack);
		stack_free(v_stack);
		return R_OK;
	}
	else {
		stack_free(v_stack);
		return R_INVALID_EXPRESSION;
	}
}

enum retcode inf_eval_outq(int *result, d_array *outq)
{
	struct stack *v_stack = stack_new(MIN_STACK_SIZE);
	struct token *array = da_get_array(outq);
	size_t elem_num = da_get_size(outq);

	if (v_stack == NULL) {
		return R_MEMORY_ALLOC;
	}

	for (size_t i = 0; i < elem_num; i++) {
		switch (array[i].type) {
			case VALUE:
				if (stack_push(v_stack, array[i].value) == R_MEMORY_ALLOC) {
					stack_free(v_stack);
					return R_MEMORY_ALLOC;
				}
				break;

			case OP_ADD:
			case OP_SUB:
			case OP_MULT:
			case OP_DIV:
			{
				enum retcode ret = pn_eval_binary_op(v_stack, array[i].type,
													 PCALC_REVERSED);

				if (ret != R_OK) {
					stack_free(v_stack);
					return ret;
				}
				break;
			}

			default:
				assert(0);
		}
	}

	if (stack_size(v_stack) == 1) {
		*result = stack_pop(v_stack);
		stack_free(v_stack);
		return R_OK;
	}
	else {
		stack_free(v_stack);
		return R_INVALID_EXPRESSION;
	}
}

// Shunting yard algorithm
enum retcode inf_eval_str(int *result, char **errp, char *expr, int *last_ans)
{
	struct stack *op_stack = stack_new(MIN_STACK_SIZE);
	d_array *outq = da_new(sizeof(struct token), MIN_STACK_SIZE);

	if (op_stack == NULL || outq == NULL)
		return R_MEMORY_ALLOC;

	*errp = expr;

	while (isspace(**errp))
		*errp += 1;

	while (**errp != '\0') {
		struct token token;
		enum retcode ret = read_token(&token, *errp, errp, last_ans);

		if (ret == R_OK) {
			switch (token.type) {
				case VALUE:
					da_append(outq, &token);
					break;

				case OP_ADD:
				case OP_SUB:
				case OP_MULT:
				case OP_DIV:
					while (stack_size(op_stack) > 0) {
						enum token_type op2 = stack_peek(op_stack);

						if (op_cmp(token.type, op2) <= 0) {
							struct token token;

							token.type = stack_pop(op_stack);
							da_append(outq, &token);
						}
						else {
							break;
						}
					}

					stack_push(op_stack, token.type);
					break;

				default:
					assert(0);
			}

			*errp += 1;
			while (isspace(**errp))
				*errp += 1;
		}
		else {
			stack_free(op_stack);
			da_free(&outq);

			switch (ret) {
				case R_UKNOWN_TOKEN:	return R_UKNOWN_TOKEN;
				case R_OUT_OF_BOUNDS:	return R_OUT_OF_BOUNDS;
				case R_NO_LAST_ANS:		return R_NO_LAST_ANS;

				default:   assert(0);
			}
		}
	}

	while (stack_size(op_stack) > 0) {
		struct token token;

		token.type = stack_pop(op_stack);
		da_append(outq, &token);
	}

	stack_free(op_stack);

	{
		enum retcode ret = inf_eval_outq(result, outq);
		da_free(&outq);
		return ret;
	}
}
