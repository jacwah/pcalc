//
//  main.c
//
//
//  Copyright 2015 Jacob Wahlgren
//
//

#include "pcalc_prefix.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "pcalc.h"
#include "settings.h"

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

void print_number(struct settings *s, int n)
{
	switch (s->output) {
		case BASE_DECIMAL:
			printf("%d", n);
			break;

		case BASE_HEX:
			if (n == INT_MIN) {
				print_error(NULL, NULL, PCALC_OUT_OF_BOUNDS);
			}
			else {
				printf("0x");

				if (n < 0) {
					putchar('-');
					n *= -1;
				}

				printf("%X\n", n);
			}
			break;

		default:
			assert(0);
	}
}

void usage(int exit_value)
{
	printf("Usage: pcalc [<option>...]\n"
		   "       pcalc [<option>...] <expression>\n"
		   "\n"
		   "       -i  infix notation (default)\n"
		   "       -r  postfix notation (rpn)\n"
		   "       -p  prefix notation\n"
		   "       -c  print config path and exit\n"
		   "       -w  print settings and exit\n"
		   "       -h  show this help\n"
		   );

	exit(exit_value);
}

void parse_argv(int *argcp, char ***argvp, struct settings *s)
{
	const char *optstr =
		"r"		// postfix (rpn)
		"p"		// prefix (pn)
		"i"		// infix
		"c"		// print config path
		"w"		// print settings
		"h"		// show help
		;
	int c;

	while ((c = getopt(*argcp, *argvp, optstr)) != -1)
		switch (c) {
			case 'r':
				s->notation = POSTFIX;
				break;

			case 'p':
				s->notation = PREFIX;
				break;

			case 'i':
				s->notation = INFIX;
				break;

			case 'c':
			{
				char path[PATH_MAX];

				get_config_path(path);
				printf("%s\n", path);

				exit(EXIT_SUCCESS);
			}

			case 'w':
				write_settings(s, stdout);

				exit(EXIT_SUCCESS);

			case 'h':
				usage(EXIT_SUCCESS);

			case '?':
			default:
				usage(EXIT_FAILURE);
		}

	*argcp -= optind - 1;
	*argvp += optind - 1;
}

int prompt_loop(struct settings *s)
{
	char *prompt = "pcalc>";
	int result;
	int *last_ans = NULL;

	switch (s->notation) {
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

				switch (s->notation) {
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
					print_number(s, result);
					last_ans = &result;
				}
				else {
					print_error(expr, errp, ret);
				}
			}
		}
		else {
			free(expr);

			if (feof(stdin)) {
				putc('\n', stdout);
				return EXIT_SUCCESS;
			}
			else {
				perror("Reading input failed");
				return EXIT_FAILURE;
			}
		}
	}
}

int main(int argc, char **argv)
{
	struct settings settings;

	read_settings(&settings);
	parse_argv(&argc, &argv, &settings);

	if (argc == 1) {
		return prompt_loop(&settings);
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

		switch (settings.notation) {
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
			print_number(&settings, result);
			return EXIT_SUCCESS;
		}
		else {
			print_error(str, errp, ret);
			return EXIT_FAILURE;
		}
	}
}
