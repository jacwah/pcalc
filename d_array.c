//
//  dynamic_array.c
//  
//
//  Copyright 2015 Jacob Wahlgren
//
//

#include <stdlib.h>
#include <string.h>
#include "d_array.h"

struct d_array {
	void  *array;
	size_t size;
	size_t elem_num;
	size_t elem_size;
};

void da_init(d_array *da, size_t elem_size, size_t initial_count)
{
	da->size = elem_size * initial_count;
	da->elem_num = 0;
	da->elem_size = elem_size;
	da->array = malloc(da->size);
}

d_array *da_new(size_t elem_size, size_t initial_count)
{
	d_array *da = malloc(sizeof(d_array));

	if (da)
		da_init(da, elem_size, initial_count);

	return da;
}

void da_free(d_array **dapp)
{
	if (dapp && *dapp) {
		free((*dapp)->array);
		free(*dapp);
		*dapp = NULL;
	}
}

size_t da_get_size(d_array *da)
{
	return da->elem_num;
}

d_array *da_set_size(d_array *da, size_t max_num)
{
	void *new_array = realloc(da->array, max_num * da->elem_size);

	if (new_array) {
		if (da->elem_num > max_num)
			da->elem_num = max_num;

		da->size = max_num * da->elem_size;
		da->array = new_array;
		return da;
	}
	else {
		return NULL;
	}
}

d_array *da_append(d_array *da, void *elem)
{
	if (da->elem_num * da->elem_size >= da->size)
		if (da_set_size(da, da->elem_num * 2) == NULL)
			return NULL;

	memcpy((char *) da->array + da->elem_num * da->elem_size,
		   elem, da->elem_size);
	da->elem_num++;

	return da;
}

void *da_get_array(d_array *da)
{
	return da->array;
}
