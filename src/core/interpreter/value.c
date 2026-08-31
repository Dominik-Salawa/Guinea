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
    v.current_value_type = header_type;
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
        case GINSTRDATATYPE_NULL:       break;
        case GINSTRDATATYPE_NUMBER32:   break;
        case GINSTRDATATYPE_NUMBER64:   break;
        default:
            printf("idk what datatype this is! %d\n", x->current_value_type);
            exit(1);
    }
    *x = (GUIN_ValueHeader){0};
}

GUIN_STATUS GUIN_assign_ValueHeader_with_ValueHeader(GUIN_ValueHeader* to_assign, GUIN_ValueHeader value)
{
    if (to_assign->header_type != value.current_value_type && to_assign->header_type != GINSTRDATATYPE_DYNAMIC) return GUIN_FAIL;

    GUIN_destroy_ValueHeader(to_assign);
    *to_assign = value;

    return GUIN_SUCCESS;
}

#include <string.h>
// make sure source starts on the datatype
GUIN_VH_from_BC_result GUIN_get_ValueHeader_from_Bytecode(GUIN_Bytecode* bytecode, GUIN_ubyte* src)
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
            if (!GUIN_safe_memcpy_Bytecode(&vh.i32, bytecode, src, sizeof(GUIN_int32))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            src += sizeof(GUIN_int32);
            break;

        case GINSTRDATATYPE_INT64:
            if (!GUIN_safe_memcpy_Bytecode(&vh.i64, bytecode, src, sizeof(GUIN_int64))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            src += sizeof(GUIN_int64);
            break;

        case GINSTRDATATYPE_NUMBER32:
            if (!GUIN_safe_memcpy_Bytecode(&vh.n32, bytecode, src, sizeof(GUIN_number32))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            src += sizeof(GUIN_number32);
            break;

        case GINSTRDATATYPE_NUMBER64:
            if (!GUIN_safe_memcpy_Bytecode(&vh.n32, bytecode, src, sizeof(GUIN_number64))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            src += sizeof(GUIN_number64);
            break;

        case GINSTRDATATYPE_CHAR:
            if (!GUIN_safe_memcpy_Bytecode(&vh.n32, bytecode, src, sizeof(char))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            ++src;
            break;

        case GINSTRDATATYPE_BOOL:
            if (!GUIN_safe_memcpy_Bytecode(&vh.n32, bytecode, src, sizeof(bool))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            ++src;
            break;
            
        case GINSTRDATATYPE_STRING: {
            GUIN_int64 len;
            if (!GUIN_safe_memcpy_Bytecode(&len, bytecode, src, sizeof(len))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            src += sizeof(len);

            vh.str = GUIN_init_String_ptr();
            if (!vh.str) {
                x.status = GUIN_MEM_FAIL;
                return x;
            }
            if (GUIN_overflow_Bytecode(bytecode, (src + len - 1))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            if (!GUIN_stringconcat_char_w_len(vh.str, (char*)src, len)) {
                x.status = GUIN_MEM_FAIL;
                return x;
            }
            src += len;
        }
        break;

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
