//
// settings.c
//
//
// Copyright 2015 Jacob Wahlgren
//
//

#include "pcalc_prefix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "settings.h"
#include "pcalc.h"

int sstrcmp(const char *a, const char *b)
{
	return strncmp(a, b, strlen(b));
}

// Is not guaranteed to exist
char *get_config_path(char path[PATH_MAX])
{
	const char *dir;

	dir = getenv("HOME");

	if (dir) {
		strncpy(path, dir, PATH_MAX);
		strncat(path, "/", PATH_MAX - strlen(path));
	}
	else {
		*path = '\0';
	}

	strncat(path, ".pcalc", PATH_MAX - strlen(path));

	return path;
}

void settings_default(struct settings *s)
{
	s->notation = INFIX;
	s->output = BASE_DECIMAL;
}

enum retcode read_notation(struct settings *s, char *arg)
{
	if (sstrcmp(arg, "infix") == 0)
		s->notation = INFIX;
	else if (sstrcmp(arg, "postfix") == 0)
		s->notation = POSTFIX;
	else if (sstrcmp(arg, "prefix") == 0)
		s->notation = PREFIX;
	else
		return PCALC_INVALID_EXPRESSION;

	return PCALC_OK;
}

enum retcode read_output(struct settings *s, char *arg)
{
	if (sstrcmp(arg, "decimal") == 0)
		s->output = BASE_DECIMAL;
	else if (sstrcmp(arg, "hex") == 0)
		s->output = BASE_HEX;
	else
		return PCALC_INVALID_EXPRESSION;

	return PCALC_OK;
}

enum retcode parse_line(struct settings *s, char *line)
{
	struct read_cmd {
		char *cmd;
		enum retcode (*func)(struct settings *s, char *arg);
	};
	struct read_cmd cmds[] = {
		{"notation", read_notation},
		{"output", read_output}
	};
	int error = 0;
	size_t i;

	while (isspace(*line))
		line++;

	if (*line == '#' || *line == '\0')
		return PCALC_OK;

	for (i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
		char *cmd = cmds[i].cmd;
		size_t len = strlen(cmd);

		if (strncmp(line, cmd, len) == 0) {
			line += len;

			if (isspace (*line++)) {
				while (isspace(*line))
					line++;

				if ((cmds[i].func)(s, line) != PCALC_OK)
					error = 1;
			}
			else {
				error = 1;
			}

			break;
		}
	}

	if (i == sizeof(cmds) / sizeof(cmds[0]))
		error = 1;	// No match

	if (error)
		return PCALC_INVALID_EXPRESSION;
	else
		return PCALC_OK;
}

void read_settings(struct settings *s)
{
	FILE *file;
	char path[PATH_MAX];

	settings_default(s);
	get_config_path(path);

	file = fopen(path, "r");

	if (file) {
		char *line = NULL;
		size_t len = 0;
		size_t lineno = 0;

		while (getline(&line, &len, file) != -1) {
			enum retcode ret = parse_line(s, line);

			lineno++;

			if (ret != PCALC_OK) {
				fprintf(stderr, "Warning: Error in settings file at line %zu\n", lineno);
			}
		}

		free(line);

		if (ferror(file)) {
			perror("Warning: Error while reading settíngs file");
		}
	}
	else {
		perror("Warning: Error while opening settings file");
	}

}

void write_settings(struct settings *s, FILE *stream)
{
	char *not_str = "";
	char *output_str = "";

	switch (s->notation) {
		case INFIX:   not_str = "infix";
		case PREFIX:  not_str = "prefix";
		case POSTFIX: not_str = "postfix";
	}

	switch (s->output) {
		case BASE_DECIMAL:	output_str = "decimal";
		case BASE_HEX:		output_str = "hex";
	}

	fprintf(stream, "notation %s\n"
					"output %s\n",
					not_str, output_str);
}
