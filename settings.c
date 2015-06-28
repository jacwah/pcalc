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
}

enum retcode parse_line(struct settings *s, char *line)
{
	int error = 0;

	if (*line == '#')
		return PCALC_OK;

	while (isspace(*line))
		line++;

	if (*line == '\0')
		return PCALC_OK;

	if (strncmp(line, "notation", strlen("notation")) == 0) {
		line += strlen("notation");

		if (isspace(*line++)) {
			while (isspace(*line))
				line++;

			if (*line == '\n')
				;
			else if (sstrcmp(line, "infix") == 0)
				s->notation = INFIX;
			else if (sstrcmp(line, "postfix") == 0)
				s->notation = POSTFIX;
			else if (sstrcmp(line, "prefix") == 0)
				s->notation = PREFIX;
			else
				error = 1;
		}
		else {
			error = 1;
		}
	}
	else {
		error = 1;
	}

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

	switch (s->notation) {
		case INFIX:   not_str = "infix";
		case PREFIX:  not_str = "prefix";
		case POSTFIX: not_str = "postfix";
	}

	fprintf(stream, "notation %s\n",
					not_str);
}
