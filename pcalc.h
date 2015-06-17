//
//  pcalc.h
//  
//
//  Copyright 2015 Jacob Wahlgren
//
//

#ifndef PCALC_H
#define PCALC_H

#define PCALC_REVERSED 1

enum retcode {
	R_OK,
	R_MEMORY_ALLOC,
	R_OUT_OF_BOUNDS,
	R_NOT_ENOUGH_VALUES,
	R_UKNOWN_TOKEN,
	R_INVALID_EXPRESSION
};

enum retcode pn_eval_str(int *result, char *expr, int is_reversed);

#endif
