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
	PCALC_OK,
	PCALC_MEMORY_ALLOC,
	PCALC_OUT_OF_BOUNDS,
	PCALC_NOT_ENOUGH_VALUES,
	PCALC_UKNOWN_TOKEN,
	PCALC_INVALID_EXPRESSION,
	PCALC_NO_LAST_ANS
};

enum retcode pn_eval_str(int *result, char **errp, char *expr,
						 int is_reversed, int *last_ans);
enum retcode inf_eval_str(int *result, char **errp, char *expr, int *last_ans);

#endif
