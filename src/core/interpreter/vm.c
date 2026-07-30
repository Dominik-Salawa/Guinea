#ifndef GUIN_VM_C
#define GUIN_VM_C

#include "stacks.h"
#include "vm.h"
#include "gc.h"
#include <math.h>
#include <string.h>

GUIN_GLOBALMAP GUIN_init_GLOBALMAP(void)
{
    GUIN_GLOBALMAP x;
    x.size = 8;
    x.length = 0;
    x.content = malloc(sizeof(GUIN_VARIABLE_HEADER) * x.size);
    if (!x.content) return (GUIN_GLOBALMAP){0};
    return x;
}
void GUIN_destroy_GLOBALMAP(GUIN_GLOBALMAP* x)
{
    if (!x) return;
    if (x->content) {
        for (size_t i = 0; i < x->length; ++i)
            GUIN_destroy_VARIABLE_HEADER(&x->content[i]);
        free(x->content);
    }
    *x = (GUIN_GLOBALMAP){0};
}
GUIN_STATUS GUIN_add_GLOBALNAME_to_GLOBALMAP(GUIN_GLOBALMAP* x, GINSTR_Datatype datatype, char* global_name)
{
    if (!x || datatype == GINSTRDATATYPE_NULL || !global_name) return GUIN_FAIL;
    if (GUIN_fetch_GLOBALNAME_from_GLOBALMAP(x, global_name))  return GUIN_FAIL;

    if (x->length >= x->size) {
        size_t original_size = x->size;
        if (x->size > 0) {
            while (x->length >= x->size) x->size *= 2;
        } else {
            x->size = 8;
        }
        GUIN_VARIABLE_HEADER* tmp = realloc(x->content, sizeof(GUIN_VARIABLE_HEADER) * x->size);
        if (!tmp) {
            x->size = original_size;
            return GUIN_MEM_FAIL;
        }
        x->content = tmp;
    }

    GUIN_VARIABLE_HEADER y = GUIN_init_VARIABLE_HEADER(datatype, global_name, NULL);
    if (!y.name) return GUIN_MEM_FAIL;
    x->content[x->length++] = y;
    return GUIN_SUCCESS;
}
GUIN_STATUS GUIN_add_GLOBALNAME_to_GLOBALMAP_cp_global_name(GUIN_GLOBALMAP* x, GINSTR_Datatype datatype, char* global_name)
{
    if (!x || datatype == GINSTRDATATYPE_NULL || !global_name) return GUIN_FAIL;
    if (GUIN_fetch_GLOBALNAME_from_GLOBALMAP(x, global_name))  return GUIN_FAIL;

    if (x->length >= x->size) {
        size_t original_size = x->size;
        if (x->size > 0) {
            while (x->length >= x->size) x->size *= 2;
        } else {
            x->size = 8;
        }
        GUIN_VARIABLE_HEADER* tmp = realloc(x->content, sizeof(GUIN_VARIABLE_HEADER) * x->size);
        if (!tmp) {
            x->size = original_size;
            return GUIN_MEM_FAIL;
        }
        x->content = tmp;
    }

    GUIN_VARIABLE_HEADER y = GUIN_init_VARIABLE_HEADER_cp_name(datatype, global_name, NULL);
    if (!y.name) return GUIN_MEM_FAIL;
    x->content[x->length++] = y;
    return GUIN_SUCCESS;
}
// DO NOT FREE
GUIN_VARIABLE_HEADER* GUIN_fetch_GLOBALNAME_from_GLOBALMAP(GUIN_GLOBALMAP* x, char* global_name)
{
    if (!x || !global_name) return NULL;

    for (size_t i = 0; i < x->length; ++i) {
        if (strcmp(x->content[i].name, global_name) == 0)
            return &x->content[i];
    }
    return NULL;
}



GUIN_VM GUIN_init_VM(void)
{
    GUIN_VM x = (GUIN_VM){0};
    x.stack_main = GUIN_init_VALUE_STACK();
    if (!x.stack_main.baseptr) {
        return (GUIN_VM){0};
    }
    x.stack_frames = GUIN_init_FRAME_STACK();
    if (!x.stack_frames.baseptr) {
        GUIN_destroy_VALUE_STACK(&x.stack_main);
        return (GUIN_VM){0};
    }
    x.gc = GUIN_init_GC();
    if (!x.gc.pages) {
        GUIN_destroy_VALUE_STACK(&x.stack_main);
        GUIN_destroy_FRAME_STACK(&x.stack_frames);
        return (GUIN_VM){0};
    }
    return x;
}
GUIN_VM* GUIN_init_VM_ptr(void)
{
    GUIN_VM* x = malloc(sizeof(GUIN_VM));
    if (!x) return NULL;
    *x = GUIN_init_VM();
    if (!x->stack_main.baseptr) // means something went wrong and the entire value got zeroed
        return NULL;
    return x;
}
void GUIN_destroy_VM(GUIN_VM* x)
{
    if (!x) return;
    GUIN_destroy_VALUE_STACK(&x->stack_main);
    GUIN_destroy_FRAME_STACK(&x->stack_frames);
    GUIN_destroy_GC(&x->gc);
    *x = (GUIN_VM){0};
}
void GUIN_destroy_VM_ptr(GUIN_VM** x)
{
    if (!x) return;
    if (!*x) return;
    GUIN_destroy_VM(*x);
    free(*x);
    *x = NULL;
}



// ARITHMETIC
bool GUIN_VM_add(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new;
    // number/int + number/int
    if (GUIN_is_number_variant(x.current_value_type) && GUIN_is_number_variant(y.current_value_type)) {
        if (GUIN_is_number(x.current_value_type) && GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = xval + yval;
        }
        else if (GUIN_is_number(x.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.n64 = xval + yval;
        }
        else if (GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = xval + yval;
        }
        else {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_INT64};
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.i64 = xval + yval;
        }
    }
    // string + string
    else if (x.current_value_type == GINSTRDATATYPE_STRING && y.current_value_type == GINSTRDATATYPE_STRING) {
        GUIN_stringconcat(x.str, y.str);
        new = x;
        GUIN_clearstring_ptr(&y.str);
    }
    // error add
    else {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_sub(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new;
    // number/int + number/int
    if (GUIN_is_number_variant(x.current_value_type) && GUIN_is_number_variant(y.current_value_type)) {
        if (GUIN_is_number(x.current_value_type) && GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = xval - yval;
        }
        else if (GUIN_is_number(x.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.n64 = xval - yval;
        }
        else if (GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = xval - yval;
        }
        else {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_INT64};
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.i64 = xval - yval;
        }
    }
    // error sub
    else {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_mul(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new;
    // number/int * number/int
    if (GUIN_is_number_variant(x.current_value_type) && GUIN_is_number_variant(y.current_value_type)) {
        if (GUIN_is_number(x.current_value_type) && GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = xval * yval;
        }
        else if (GUIN_is_number(x.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.n64 = xval * yval;
        }
        else if (GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = xval * yval;
        }
        else {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_INT64};
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.i64 = xval * yval;
        }
    }
    // string/int * string/int
    else if ((x.current_value_type == GINSTRDATATYPE_STRING || y.current_value_type == GINSTRDATATYPE_STRING) && (GUIN_is_int(x.current_value_type) || GUIN_is_int(y.current_value_type))) {
        GUIN_ValueHeader* hd;
        size_t loopcount;

        if (GUIN_is_int(x.current_value_type)) {
            hd = &y;
            loopcount = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32;
        } else {
            hd = &x;
            loopcount = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
        }

        new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_STRING, .str=GUIN_init_String_ptr()};
        if (new.str->size <= hd->str->length * loopcount) {
            new.str->size = hd->str->length * loopcount + 1;
            char* tmp = realloc(new.str->content, sizeof(char) * new.str->size);
            if (tmp) {
                new.str->content = tmp;
            } else {
                new.str->size = new.str->length / loopcount - 1;
            }
        }
        for (size_t i = 0; i < loopcount; ++i)
            GUIN_stringconcat(new.str, hd->str);

        GUIN_destroy_ValueHeader(hd);
    }
    // error mul
    else {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_div(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new;
    // number/int / number/int
    if (GUIN_is_number_variant(x.current_value_type) && GUIN_is_number_variant(y.current_value_type)) {
        if (GUIN_is_number(x.current_value_type) && GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = xval / yval;
        }
        else if (GUIN_is_number(x.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.n64 = xval / yval;
        }
        else if (GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = xval / yval;
        }
        else {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.n64 = xval / yval;
        }
    }
    // error div
    else {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_mod(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new;
    // number/int ^ number/int
    if (GUIN_is_number_variant(x.current_value_type) && GUIN_is_number_variant(y.current_value_type)) {
        if (GUIN_is_number(x.current_value_type) && GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = fmod(xval, yval);
        }
        else if (GUIN_is_number(x.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.n64 = fmod(xval, yval);
        }
        else if (GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = fmod(xval, yval);
        }
        else {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.n64 = xval % yval;
        }
    }
    // error pow
    else {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_pow(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new;
    // number/int ^ number/int
    if (GUIN_is_number_variant(x.current_value_type) && GUIN_is_number_variant(y.current_value_type)) {
        if (GUIN_is_number(x.current_value_type) && GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = pow(xval, yval);
        }
        else if (GUIN_is_number(x.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.n64 = pow(xval, yval);
        }
        else if (GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = pow(xval, yval);
        }
        else {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.n64 = pow(xval, yval);
        }
    }
    // error pow
    else {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_neg(GUIN_VM* vm)
{
    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        return false;
    }

    GUIN_ValueHeader new = (GUIN_ValueHeader){0};
    // number/int/bool/char == number/int/bool/char
    if (GUIN_is_number_variant(x.current_value_type)) {
        if (GUIN_is_number(x.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            new.n64 = -xval;
            new.current_value_type = GINSTRDATATYPE_NUMBER64;
        }
        else {
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            new.i64 = -xval;
            new.current_value_type = GINSTRDATATYPE_INT64;
        }
    }
    // error neg
    else {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

// LOGIC
bool GUIN_VM_not(GUIN_VM* vm)
{
    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL)
        return false;

    GUIN_ValueHeader new = (GUIN_ValueHeader){0,.bl=GUIN_is_false_eval(x),.current_value_type=GINSTRDATATYPE_BOOL};
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_and(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new = {0, .bl=(!GUIN_is_false_eval(x) && !GUIN_is_false_eval(y)), .current_value_type=GINSTRDATATYPE_BOOL};
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_or(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new = {0, .bl=(!GUIN_is_false_eval(x) || !GUIN_is_false_eval(y)), .current_value_type=GINSTRDATATYPE_BOOL};
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_equ(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_BOOL};
    // number/int/bool/char == number/int/bool/char
    if (GUIN_is_number_variant(x.current_value_type) && GUIN_is_number_variant(y.current_value_type)) {
        if (GUIN_is_number(x.current_value_type) && GUIN_is_number(y.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.bl = xval == yval;
        }
        else if (GUIN_is_number(x.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.bl = xval == yval;
        }
        else if (GUIN_is_number(y.current_value_type)) {
            GUIN_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.bl = xval == yval;
        }
        else {
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.bl = xval == yval;
        }
    }
    else if (x.current_value_type == GINSTRDATATYPE_STRING && y.current_value_type == GINSTRDATATYPE_STRING) {
        new.bl = GUIN_stringcompare(x.str, y.str);
        GUIN_clearstring_ptr(&x.str);
        GUIN_clearstring_ptr(&y.str);
    }
    else {
        new.bl = x.current_value_type == y.current_value_type;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_not_equ(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_BOOL};
    // number/int/bool/char != number/int/bool/char
    if (GUIN_is_number_variant(x.current_value_type) && GUIN_is_number_variant(y.current_value_type)) {
        if (GUIN_is_number(x.current_value_type) && GUIN_is_number(y.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.bl = xval != yval;
        }
        else if (GUIN_is_number(x.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.bl = xval != yval;
        }
        else if (GUIN_is_number(y.current_value_type)) {
            GUIN_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.bl = xval != yval;
        }
        else {
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.bl = xval != yval;
        }
    }
    else if (x.current_value_type == GINSTRDATATYPE_STRING && y.current_value_type == GINSTRDATATYPE_STRING) {
        new.bl = !GUIN_stringcompare(x.str, y.str);
        GUIN_clearstring_ptr(&x.str);
        GUIN_clearstring_ptr(&y.str);
    }
    else {
        new.bl = x.current_value_type != y.current_value_type;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_gt(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_BOOL};
    // number/int/bool/char > number/int/bool/char
    if (GUIN_is_number_variant(x.current_value_type) && GUIN_is_number_variant(y.current_value_type)) {
        if (GUIN_is_number(x.current_value_type) && GUIN_is_number(y.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.bl = xval > yval;
        }
        else if (GUIN_is_number(x.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.bl = xval > yval;
        }
        else if (GUIN_is_number(y.current_value_type)) {
            GUIN_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.bl = xval > yval;
        }
        else {
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.bl = xval > yval;
        }
    }
    else {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_lt(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_BOOL};
    // number/int/bool/char < number/int/bool/char
    if (GUIN_is_number_variant(x.current_value_type) && GUIN_is_number_variant(y.current_value_type)) {
        if (GUIN_is_number(x.current_value_type) && GUIN_is_number(y.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.bl = xval < yval;
        }
        else if (GUIN_is_number(x.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.bl = xval < yval;
        }
        else if (GUIN_is_number(y.current_value_type)) {
            GUIN_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.bl = xval < yval;
        }
        else {
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.bl = xval < yval;
        }
    }
    else {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_gt_equ(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_BOOL};
    // number/int/bool/char >= number/int/bool/char
    if (GUIN_is_number_variant(x.current_value_type) && GUIN_is_number_variant(y.current_value_type)) {
        if (GUIN_is_number(x.current_value_type) && GUIN_is_number(y.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.bl = xval >= yval;
        }
        else if (GUIN_is_number(x.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.bl = xval >= yval;
        }
        else if (GUIN_is_number(y.current_value_type)) {
            GUIN_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.bl = xval >= yval;
        }
        else {
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.bl = xval >= yval;
        }
    }
    else {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool GUIN_VM_lt_equ(GUIN_VM* vm)
{
    GUIN_ValueHeader y = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    GUIN_ValueHeader x = GUIN_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    GUIN_ValueHeader new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_BOOL};
    // number/int/bool/char <= number/int/bool/char
    if (GUIN_is_number_variant(x.current_value_type) && GUIN_is_number_variant(y.current_value_type)) {
        if (GUIN_is_number(x.current_value_type) && GUIN_is_number(y.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.bl = xval <= yval;
        }
        else if (GUIN_is_number(x.current_value_type)) {
            GUIN_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            GUIN_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.bl = xval <= yval;
        }
        else if (GUIN_is_number(y.current_value_type)) {
            GUIN_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.bl = xval <= yval;
        }
        else {
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.bl = xval <= yval;
        }
    }
    else {
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}















GUIN_STATUS GUIN_load_VM(GUIN_VM* vm, GUIN_Bytecode* bytecode)
{
    for (size_t i = 0; i < bytecode->length;) {
        switch (bytecode->bytecode[i])
        {
            case GINSTR_PUSH_IMMEDIATE: {
                ++i;
                GUIN_VH_from_BC_result x = GUIN_get_ValueHeader_from_Bytecode(&bytecode->bytecode[i]);
                if (x.status == GUIN_FAIL) {
                    GUIN_printf("%sc\n", x.errmsg);
                    GUIN_destroy_ValueHeader(&x.value);
                    return GUIN_FAIL;
                }
                if (x.status == GUIN_MEM_FAIL) {
                    GUIN_destroy_ValueHeader(&x.value);
                    return GUIN_MEM_FAIL;
                }

                bool s = GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x.value);
                if (!s) {
                    GUIN_destroy_ValueHeader(&x.value);
                    return s;
                }
                i += x.to_jump;
                break;
            }

            case GINSTR_ADD:
                if (!GUIN_VM_add(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_SUB:
                if (!GUIN_VM_sub(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_MUL:
                if (!GUIN_VM_mul(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_DIV:
                if (!GUIN_VM_div(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_MOD:
                if (!GUIN_VM_mod(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_POW:
                if (!GUIN_VM_pow(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_NEG:
                if (!GUIN_VM_neg(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_NOT:
                if (!GUIN_VM_not(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_EQU:
                if (!GUIN_VM_equ(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_NOT_EQU:
                if (!GUIN_VM_not_equ(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_GT:
                if (!GUIN_VM_gt(vm))
                    return GUIN_FAIL;
                ++i;
                break;
            
            case GINSTR_LT:
                if (!GUIN_VM_lt(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_GT_EQU:
                if (!GUIN_VM_gt_equ(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_LT_EQU:
                if (!GUIN_VM_lt_equ(vm))
                    return GUIN_FAIL;
                ++i;
                break;

            case GINSTR_DECLARE_GLOBAL: {
                ++i;
                GINSTR_Datatype dt = bytecode->bytecode[i];
                ++i;
                GUIN_uint64 namelen;
                memcpy(&namelen, &bytecode->bytecode[i], sizeof(namelen));
                i += sizeof(namelen);
                char* name = malloc(sizeof(char) * namelen + 1);
                if (!name) return GUIN_MEM_FAIL;

                for (size_t j = 0; j < (size_t)namelen; ++j, ++i)
                    name[j] = bytecode->bytecode[i];
                name[namelen] = 0;

                GUIN_STATUS s = GUIN_add_GLOBALNAME_to_GLOBALMAP(&vm->global, dt, name);
                if (s != GUIN_SUCCESS) {
                    free(name);
                    return s;
                }
                i += namelen;

                { // assigns the Global variable its own dedicated slot and assigns from stack
                    GUIN_ValueHeader** x = GUIN_GC_get_memory(vm, 1);
                    if (!x) return GUIN_MEM_FAIL;
                    
                    GUIN_VARIABLE_HEADER* glocation = GUIN_fetch_GLOBALNAME_from_GLOBALMAP(&vm->global, name);
                    glocation->ptr_to_value = *x;
                    free(x);

                    GUIN_ValueHeader popped_val = GUIN_pop_VALUE_STACK(&vm->stack_main);
                    if (popped_val.current_value_type == GINSTRDATATYPE_NULL) return GUIN_FAIL;
                    *glocation->ptr_to_value = popped_val;
                }

                break;
            }

            default:
                printf("error byte: %d %zu\n", bytecode->bytecode[i], i);
                return GUIN_FAIL;
        }
    }
    return GUIN_SUCCESS;
}

// true:  loaded and did execute (but not necessarily there was no runtime errors)
// false: failed to load/execute
GUIN_STATUS GUIN_exec_VM(GUIN_VM* vm, char* start)
{
    GUIN_VARIABLE_HEADER* start_header = GUIN_fetch_GLOBALNAME_from_GLOBALMAP(&vm->global, start);
    if (!start_header) return GUIN_FAIL;
    if (start_header->datatype != GINSTRDATATYPE_FUNCTION || !start_header->ptr_to_value) return GUIN_FAIL;

    return GUIN_SUCCESS;
}

#endif
