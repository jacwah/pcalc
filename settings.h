//
// settings.c
//
//
// Copyright 2015 Jacob Wahlgren
//
//

#ifndef SETTINGS_H
#define SETTINGS_H

#include "pcalc.h"

enum notation {
	PREFIX,
	POSTFIX,
	INFIX
};

enum base {
	BASE_DECIMAL,
	BASE_HEX
};

struct settings {
	enum notation notation;
	enum base output;
};

void read_settings(struct settings *settings);
void write_settings(struct settings *s, FILE *stream);
char *get_config_path(char path[PATH_MAX]);

#endif
