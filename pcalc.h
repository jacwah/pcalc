//
//  pcalc.h
//  
//
//  Created by Jacob Wahlgren on 2015-06-09.
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
