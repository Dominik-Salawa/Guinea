#ifndef DECLARATIONS_C
#define DECLARATIONS_C

#include "../include/declarations.h"

GUIN_number32 GUIN__NaN__(void)
{
    union {
        GUIN_uint32 i;
        GUIN_number32 n;
    } u;

    u.i = 0x7FF80000;
    return u.n;
}

#endif
