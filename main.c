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
		default: assert(0);
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
				enum retcode ret = pn_eval_str(&result, expr, 0);

				if (ret == R_OK) {
					printf("%d\n", result);
				}
				else {
					fprintf(stderr, "error: %s\n", retcode_str(ret));
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

		int result = 0;
		enum retcode ret = pn_eval_str(&result, str, PCALC_REVERSED);

		if (ret == R_OK) {
			printf("%d\n", result);
			return EXIT_SUCCESS;
		}
		else {
			fprintf(stderr, "error: %s\n", retcode_str(ret));
			return EXIT_FAILURE;
		}
	}
}
