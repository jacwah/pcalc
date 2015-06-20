//
//  dynamic_array.h
//  
//
//  Copyright 2015 Jacob Wahlgren
//
//

#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

struct d_array;
typedef struct d_array d_array;

void		da_init(d_array *da, size_t elem_size, size_t initial_count);
d_array *	da_new(size_t elem_size, size_t initial_count);
void		da_free(d_array **dapp);
size_t		da_get_size(d_array *da);
d_array *	da_set_size(d_array *da, size_t max_num);
d_array *	da_append(d_array *da, void *elem);
void *		da_get_array(d_array *da);

#endif
