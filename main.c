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
#include <unistd.h>
#include "pcalc.h"

enum notation { PREFIX, POSTFIX, INFIX };

const char *retcode_str(enum retcode ret)
{
	switch(ret) {
		case PCALC_OK:					return "No errors occurred";
		case PCALC_MEMORY_ALLOC:		return "Memory allocation failed";
		case PCALC_OUT_OF_BOUNDS:		return "Value out of bounds";
		case PCALC_NOT_ENOUGH_VALUES:	return "Not enough values";
		case PCALC_UKNOWN_TOKEN:		return "Uknown token";
		case PCALC_INVALID_EXPRESSION:	return "Invalid expression";
		case PCALC_NO_LAST_ANS:			return "No previous answer";
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

void usage()
{
	printf("usage: pcalc [-rpi]\n"
		   "       pcalc [-rpi] <expression>\n"
		   "\n"
		   "       -i  infix notation (default)\n"
		   "       -r  postfix notation (rpn)\n"
		   "       -p  prefix notation\n"
		   );
	exit(EXIT_FAILURE);
}

void parse_argv(int *argcp, char ***argvp, enum notation *notation)
{
	const char *optstr =
		"r"		// postfix (rpn)
		"p"		// prefix (pn)
		"i";	// infix
	int c;

	while ((c = getopt(*argcp, *argvp, optstr)) != -1)
		switch (c) {
			case 'r':
				*notation = POSTFIX;
				break;

			case 'p':
				*notation = PREFIX;
				break;

			case 'i':
				*notation = INFIX;
				break;

			case '?':
			default:
				usage();
		}

	*argcp -= optind - 1;
	*argvp += optind - 1;
}

int prompt_loop(enum notation notation)
{
	char *prompt = "pcalc>";
	int result;
	int *last_ans = NULL;

	switch (notation) {
		case PREFIX:
			prompt = "pcalc[p]";
			break;

		case POSTFIX:
			prompt = "pcalc[r]";
			break;

		case INFIX:
			prompt = "pcalc[i]";
			break;

		default:
			assert(0);
	}

	fprintf(stderr, "Type 'q' or 'quit' to exit\n");
	for (;;) {
		char *expr = NULL;
		size_t len = 0;

		printf("%s> ", prompt);

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
				enum retcode ret;

				switch (notation) {
					case PREFIX:
						ret = pn_eval_str(&result, &errp, expr, 0, last_ans);
						break;

					case POSTFIX:
						ret = pn_eval_str(&result, &errp, expr,
										  PCALC_REVERSED, last_ans);
						break;

					case INFIX:
						ret = inf_eval_str(&result, &errp, expr, last_ans);
						break;
						
					default:
						assert(0);
				}

				if (ret == PCALC_OK) {
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
	enum notation notation = INFIX;

	parse_argv(&argc, &argv, &notation);

	if (argc == 1) {
		return prompt_loop(notation);
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
		enum retcode ret;

		switch (notation) {
			case PREFIX:
				ret = pn_eval_str(&result, &errp, str, 0, NULL);
				break;

			case POSTFIX:
				ret = pn_eval_str(&result, &errp, str,
								  PCALC_REVERSED, NULL);
				break;

			case INFIX:
				ret = inf_eval_str(&result, &errp, str, NULL);
				break;

			default:
				assert(0);
		}

		if (ret == PCALC_OK) {
			printf("%d\n", result);
			return EXIT_SUCCESS;
		}
		else {
			print_error(str, errp, ret);
			return EXIT_FAILURE;
		}
	}
}
