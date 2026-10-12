#ifndef guin_DECLARATIONS_c
#define guin_DECLARATIONS_c

#include "../include/declarations.h"
#include <stdlib.h>

guin_number32 guin__NaN__(void)
{
    union {
        guin_uint32 i;
        guin_number32 n;
    } u;

    u.i = 0x7FF80000;
    return u.n;
}

void* guin_malloc(size_t size)
	{ return malloc(size); }
void* guin_remalloc(void* ptr, size_t size)
	{ return realloc(ptr, size); }
void  guin_mfree(void* ptr)
	{ free(ptr); }

#endif
