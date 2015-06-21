//
//  main.c
//  
//
//  Copyright 2015 Jacob Wahlgren
//
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "pcalc.h"

const char *retcode_str(enum retcode ret)
{
	switch(ret) {
		case R_OK:					return "No errors occurred";
		case R_MEMORY_ALLOC:		return "Memory allocation failed";
		case R_OUT_OF_BOUNDS:		return "Value out of bounds";
		case R_NOT_ENOUGH_VALUES:	return "Not enough values";
		case R_UKNOWN_TOKEN:		return "Uknown token";
		case R_INVALID_EXPRESSION:	return "Invalid expression";
		case R_NO_LAST_ANS:			return "No previous answer";
		default: assert(0);
	}
}

void print_error(char *expr, char *errp, enum retcode ret)
{
	if (expr && errp) {
		assert (errp >= expr);

		if (expr[strlen(expr) - 1] == '\n')
			expr[strlen(expr) - 1] = '\0';

		fprintf(stderr,
				"Error: %s\n"
				"\t%s\n"
				"\t",
				retcode_str(ret), expr);

		for (size_t i = 0; i < errp - expr; i++)
			fputc(' ', stderr);

		fprintf(stderr, "^\n");
	}
	else {
		fprintf(stderr, "Error: %s\n", retcode_str(ret));
	}
}

int prompt_loop()
{
	int result;
	int *last_ans = NULL;

	fprintf(stderr, "Type 'q' or 'quit' to exit\n");
	for (;;) {
		char *expr = NULL;
		size_t len = 0;

		printf("pcalc> ");

		if (getline(&expr, &len, stdin) > 0) {
			if (expr[0] == '\n') {
				continue;
			}
			else if (strcmp(expr, "q\n") == 0 || strcmp(expr, "quit\n") == 0) {
				free(expr);
				return EXIT_SUCCESS;
			}
			else {
				char *errp;
				enum retcode ret = inf_eval_str(&result, &errp, expr,
											    last_ans);

				if (ret == R_OK) {
					printf("%d\n", result);
					last_ans = &result;
				}
				else {
					print_error(expr, errp, ret);
				}
			}
		}
		else {
			free(expr);
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

		int result = 0;
		char *errp;
		enum retcode ret = inf_eval_str(&result, &errp, str, NULL);

		if (ret == R_OK) {
			printf("%d\n", result);
			return EXIT_SUCCESS;
		}
		else {
			print_error(str, errp, ret);
			return EXIT_FAILURE;
		}
	}
}
