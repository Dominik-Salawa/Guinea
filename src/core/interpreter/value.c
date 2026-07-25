#ifndef GUIN_VALUE_C
#define GUIN_VALUE_C

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../etc/strings.h"
#include "value.h"

#include <stdlib.h>
#include <stdio.h>

GUIN_ValueHeader GUIN_init_ValueHeader(GINSTR_Datatype header_type)
{
    GUIN_ValueHeader v = {0};
    v.header_type = header_type;
    return v;
}
void GUIN_destroy_ValueHeader(GUIN_ValueHeader* x)
{
    switch (x->current_value_type)
    {
        case GINSTRDATATYPE_STRING:
            if (x->str) GUIN_clearstring_ptr(&x->str);
            break;
        case GINSTRDATATYPE_INT32:      break;   
        case GINSTRDATATYPE_INT64:      break;   
        case GINSTRDATATYPE_BOOL:       break;   
        case GINSTRDATATYPE_CHAR:       break;   
        case GINSTRDATATYPE_NIL:        break;   
        case GINSTRDATATYPE_NUMBER32:   break;   
        case GINSTRDATATYPE_NUMBER64:   break;   
        default:
            printf("idk what datatype this is! %d\n", x->current_value_type);
            exit(1);
    }
    *x = (GUIN_ValueHeader){0};
}

#endif
