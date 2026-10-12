#ifndef guin_VALUE_c
#define guin_VALUE_c

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../etc/strings.h"
#include "value.h"
#include "vm.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

guin_ValueHeader guin_init_ValueHeader(GINSTR_Datatype header_type)
{
    guin_ValueHeader v = {0};
    v.header_type = header_type;
    v.current_value_type = header_type;
    return v;
}
void guin_destroy_ValueHeader(guin_ValueHeader* x)
{
    switch (x->current_value_type)
    {
        case GINSTRDATATYPE_STRING:
            if (x->str) guin_clearstring_ptr(&x->str);
            break;
        case GINSTRDATATYPE_INT32:      break;
        case GINSTRDATATYPE_INT64:      break;
        case GINSTRDATATYPE_BOOL:       break;
        case GINSTRDATATYPE_CHAR:       break;
        case GINSTRDATATYPE_NIL:        break;
        case GINSTRDATATYPE_NULL:       break;
        case GINSTRDATATYPE_NUMBER32:   break;
        case GINSTRDATATYPE_NUMBER64:   break;
		case GINSTRDATATYPE_DYNAMIC:    break;
    }
    *x = (guin_ValueHeader){0};
}


guin_STATUS guin_assign_ValueHeader_with_ValueHeader(guin_ValueHeader* to_assign, guin_ValueHeader value)
{
    if (to_assign->header_type != value.current_value_type && to_assign->header_type != GINSTRDATATYPE_DYNAMIC && value.current_value_type != GINSTRDATATYPE_NIL) {
		if (guin_is_number(to_assign->header_type) && guin_is_number(value.current_value_type)) {
			guin_number64 val = (value.current_value_type == GINSTRDATATYPE_NUMBER64)? value.n64 : value.n32;
			if (to_assign->header_type == GINSTRDATATYPE_NUMBER64)
				to_assign->n64 = val;
			else
				to_assign->n32 = val;
		}
		else if (guin_is_number(to_assign->header_type) && guin_is_int(value.current_value_type)) {
			guin_int64 val = (value.current_value_type == GINSTRDATATYPE_INT64)? value.i64 : value.i32;
			if (to_assign->header_type == GINSTRDATATYPE_NUMBER64)
				to_assign->n64 = val;
			else
				to_assign->n32 = val;
		}
		else if (guin_is_int(to_assign->header_type) && guin_is_number(value.current_value_type)) {
			guin_number64 val = (value.current_value_type == GINSTRDATATYPE_NUMBER64)? value.n64 : value.n32;
			if (to_assign->header_type == GINSTRDATATYPE_INT64)
				to_assign->i64 = val;
			else
				to_assign->i32 = val;
		}
		else if (guin_is_int(to_assign->header_type) && guin_is_int(value.current_value_type)) {
			guin_int64 val = (value.current_value_type == GINSTRDATATYPE_INT64)? value.i64 : value.i32;
			if (to_assign->header_type == GINSTRDATATYPE_INT64)
				to_assign->i64 = val;
			else
				to_assign->i32 = val;
		} 
		else {
			return guin_FAIL;
		}
    } else {
		guin_destroy_ValueHeader(to_assign);
		*to_assign = value;
	}
    return guin_SUCCESS;
}

// make sure source starts on the datatype
guin_VH_from_BC_result guin_get_ValueHeader_from_Bytecode(guin_Bytecode* bytecode, guin_ubyte* src)
{
    guin_VH_from_BC_result x = {.status=guin_FAIL};
    guin_ubyte* original = src;
    guin_ValueHeader vh = (guin_ValueHeader){0};
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
            if (!guin_safe_memcpy_Bytecode(&vh.i32, bytecode, src, sizeof(guin_int32))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            src += sizeof(guin_int32);
            break;

        case GINSTRDATATYPE_INT64:
            if (!guin_safe_memcpy_Bytecode(&vh.i64, bytecode, src, sizeof(guin_int64))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            src += sizeof(guin_int64);
            break;

        case GINSTRDATATYPE_NUMBER32:
            if (!guin_safe_memcpy_Bytecode(&vh.n32, bytecode, src, sizeof(guin_number32))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            src += sizeof(guin_number32);
            break;

        case GINSTRDATATYPE_NUMBER64:
            if (!guin_safe_memcpy_Bytecode(&vh.n32, bytecode, src, sizeof(guin_number64))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            src += sizeof(guin_number64);
            break;

        case GINSTRDATATYPE_CHAR:
            if (!guin_safe_memcpy_Bytecode(&vh.n32, bytecode, src, sizeof(char))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            ++src;
            break;

        case GINSTRDATATYPE_BOOL:
            if (!guin_safe_memcpy_Bytecode(&vh.n32, bytecode, src, sizeof(bool))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            ++src;
            break;
            
        case GINSTRDATATYPE_STRING: {
            guin_int64 len;
            if (!guin_safe_memcpy_Bytecode(&len, bytecode, src, sizeof(len))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            src += sizeof(len);

            vh.str = guin_init_String_ptr();
            if (!vh.str) {
                x.status = guin_MEM_FAIL;
                return x;
            }
            if (guin_overflow_Bytecode(bytecode, (src + len - 1))) {
                x.errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
                return x;
            }
            if (!guin_stringconcat_char_w_len(vh.str, (char*)src, len)) {
                x.status = guin_MEM_FAIL;
                return x;
            }
            src += len;
        	break;
        }

		case GINSTRDATATYPE_FUNCTION: {
            guin_int64 len;
		}
    }
    x.to_jump = src - original;
    x.value = vh;
    x.status = guin_SUCCESS;
    return x;
}


#endif
