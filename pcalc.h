//
//  pcalc.h
//  
//
//  Copyright 2015 Jacob Wahlgren
//
//

#ifndef PCALC_H
#define PCALC_H

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
