#ifndef VALUE_C
#define VALUE_C

#include <inttypes.h>
#include <stdlib.h>
#include "../etc/strings.h"
#include "value.h"

G_Value init_Value()
{
    G_Value v;
    v.type         = G_VALUETYPE_DEAD;
    v.str.content  = NULL;
    v.str.length   = 0;
    v.str.size     = 0;
    return v;
}

#endif