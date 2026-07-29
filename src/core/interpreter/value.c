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

#include <string.h>
// make sure source starts on the datatype
GUIN_VH_from_BC_result GUIN_get_ValueHeader_from_Bytecode(GUIN_ubyte* src)
{
    GUIN_VH_from_BC_result x = {.status=GUIN_FAIL};
    GUIN_ubyte* original = src;
    GUIN_ValueHeader vh = (GUIN_ValueHeader){0};
    if (!src) return x;
    vh.current_value_type = *src++;
    vh.header_type = vh.current_value_type;
    if (vh.current_value_type == GINSTRDATATYPE_NULL) {
        x.errmsg = "Cannot have a value with datatype NULL!";
        return x;
    }

    switch (vh.current_value_type)
    {
        case GINSTRDATATYPE_NIL:
            break;

        case GINSTRDATATYPE_INT32:
            memcpy(&vh.i32, src, sizeof(GUIN_int32));
            src += sizeof(GUIN_int32);
            break;

        case GINSTRDATATYPE_INT64:
            memcpy(&vh.i64, src, sizeof(GUIN_int64));
            src += sizeof(GUIN_int64);
            break;

        case GINSTRDATATYPE_NUMBER32:
            memcpy(&vh.n32, src, sizeof(GUIN_number32));
            src += sizeof(GUIN_number32);
            break;

        case GINSTRDATATYPE_NUMBER64:
            memcpy(&vh.n64, src, sizeof(GUIN_number64));
            src += sizeof(GUIN_number64);
            break;

        case GINSTRDATATYPE_CHAR:
            vh.ch = *((char*)src);
            ++src;
            break;

        case GINSTRDATATYPE_BOOL:
            vh.bl = *((bool*)src);
            ++src;
            break;
            
        case GINSTRDATATYPE_STRING: {
            GUIN_int64 len;
            memcpy(&len, src, sizeof(GUIN_int64));
            src += sizeof(len);

            vh.str = GUIN_init_String_ptr();
            if (!vh.str) {
                x.status = GUIN_MEM_FAIL;
                return x;
            }
            if (!GUIN_stringconcat_char_w_len(vh.str, (char*)src, len)) {
                x.status = GUIN_MEM_FAIL;
                return x;
            }
            src += len;
            break;
        }

        default:
            x.errmsg = "Unsupported datatype";
            return x;
    }
    x.to_jump = src - original;
    x.value = vh;
    x.status = GUIN_SUCCESS;
    return x;
}


#endif
