//
//  pcalc.h
//  
//
//  Copyright 2015 Jacob Wahlgren
//
//

#ifndef PCALC_H
#define PCALC_H

enum retcode {
	R_OK,
	R_MEMORY_ALLOC,
	R_OUT_OF_BOUNDS,
	R_NOT_ENOUGH_VALUES,
	R_UKNOWN_TOKEN,
	R_INVALID_EXPRESSION
};

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

#endif
