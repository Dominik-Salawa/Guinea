#ifndef guin_VM_c
#define guin_VM_c

#include "stacks.h"
#include "vm.h"
#include "gc.h"
#include <math.h>
#include <string.h>

guin_GLOBALMAP guin_init_GLOBALMAP(void)
{
    guin_GLOBALMAP x;
    x.size = 8;
    x.length = 0;
    x.content = malloc(sizeof(guin_VARIABLE_HEADER) * x.size);
    if (!x.content) return (guin_GLOBALMAP){0};
    return x;
}
void guin_destroy_GLOBALMAP(guin_GLOBALMAP* x)
{
    if (!x) return;
    if (x->content) {
        for (size_t i = 0; i < x->length; ++i)
            guin_destroy_VARIABLE_HEADER(&x->content[i]);
        free(x->content);
    }
    *x = (guin_GLOBALMAP){0};
}
guin_STATUS guin_add_GLOBALNAME_to_GLOBALMAP(guin_GLOBALMAP* x, GINSTR_Datatype datatype, char* global_name)
{
    if (!x || datatype == GINSTRDATATYPE_NULL || !global_name) return guin_FAIL;
    if (guin_fetch_GLOBALNAME_from_GLOBALMAP(x, global_name))  return guin_FAIL;

    if (x->length >= x->size) {
        size_t original_size = x->size;
        if (x->size > 0) {
            while (x->length >= x->size) x->size *= 2;
        } else {
            x->size = 8;
        }
        guin_VARIABLE_HEADER* tmp = realloc(x->content, sizeof(guin_VARIABLE_HEADER) * x->size);
        if (!tmp) {
            x->size = original_size;
            return guin_MEM_FAIL;
        }
        x->content = tmp;
    }

    guin_VARIABLE_HEADER y = guin_init_VARIABLE_HEADER(datatype, global_name, NULL);
    if (!y.name) return guin_MEM_FAIL;
    x->content[x->length++] = y;
    return guin_SUCCESS;
}
guin_STATUS guin_add_GLOBALNAME_to_GLOBALMAP_cp_global_name(guin_GLOBALMAP* x, GINSTR_Datatype datatype, char* global_name)
{
    if (!x || datatype == GINSTRDATATYPE_NULL || !global_name) return guin_FAIL;
    if (guin_fetch_GLOBALNAME_from_GLOBALMAP(x, global_name))  return guin_FAIL;

    if (x->length >= x->size) {
        size_t original_size = x->size;
        if (x->size > 0) {
            while (x->length >= x->size) x->size *= 2;
        } else {
            x->size = 8;
        }
        guin_VARIABLE_HEADER* tmp = realloc(x->content, sizeof(guin_VARIABLE_HEADER) * x->size);
        if (!tmp) {
            x->size = original_size;
            return guin_MEM_FAIL;
        }
        x->content = tmp;
    }

    guin_VARIABLE_HEADER y = guin_init_VARIABLE_HEADER_cp_name(datatype, global_name, NULL);
    if (!y.name) return guin_MEM_FAIL;
    x->content[x->length++] = y;
    return guin_SUCCESS;
}
// DO NOT FREE
guin_VARIABLE_HEADER* guin_fetch_GLOBALNAME_from_GLOBALMAP(guin_GLOBALMAP* x, char* global_name)
{
    if (!x || !global_name) return NULL;

    for (size_t i = 0; i < x->length; ++i) {
        if (strcmp(x->content[i].name, global_name) == 0)
            return &x->content[i];
    }
    return NULL;
}



guin_VM guin_init_VM(void)
{
    guin_VM x = (guin_VM){0};
    x.stack_main = guin_init_VALUE_STACK();
    if (!x.stack_main.baseptr) {
        return (guin_VM){0};
    }
    x.stack_frames = guin_init_FRAME_STACK();
    if (!x.stack_frames.baseptr) {
        guin_destroy_VALUE_STACK(&x.stack_main);
        return (guin_VM){0};
    }
    x.gc = guin_init_GC();
    if (!x.gc.pages) {
        guin_destroy_VALUE_STACK(&x.stack_main);
        guin_destroy_FRAME_STACK(&x.stack_frames);
        return (guin_VM){0};
    }
    return x;
}
guin_VM* guin_init_VM_ptr(void)
{
    guin_VM* x = malloc(sizeof(guin_VM));
    if (!x) return NULL;
    *x = guin_init_VM();
    if (!x->stack_main.baseptr) // means something went wrong and the entire value got zeroed
        return NULL;
    return x;
}
void guin_destroy_VM(guin_VM* x)
{
    if (!x) return;
    guin_destroy_VALUE_STACK(&x->stack_main);
    guin_destroy_FRAME_STACK(&x->stack_frames);
    guin_destroy_GC(&x->gc);
    *x = (guin_VM){0};
}
void guin_destroy_VM_ptr(guin_VM** x)
{
    if (!x) return;
    if (!*x) return;
    guin_destroy_VM(*x);
    free(*x);
    *x = NULL;
}

#define guin_VM_integer_branch(x,y, both_num, both_num_datatype, x_is_num, x_is_num_datatype, y_is_num, y_is_num_datatype, both_ints, both_int_datatype)\
    if (guin_is_number(x.current_value_type) && guin_is_number(y.current_value_type)) {\
        new = guin_init_ValueHeader(both_num_datatype);\
        guin_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32;\
        guin_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;\
        both_num;\
    }\
    else if (guin_is_number(x.current_value_type)) {\
        new = guin_init_ValueHeader(x_is_num_datatype);\
        guin_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32;\
        guin_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;\
        x_is_num;\
    }\
    else if (guin_is_number(y.current_value_type)) {\
        new = guin_init_ValueHeader(y_is_num_datatype);\
        guin_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32;\
        guin_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;\
        y_is_num;\
    }\
    else {\
        new = guin_init_ValueHeader(both_int_datatype);\
        guin_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32;\
        guin_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;\
        both_ints;\
    }

#define guin_VM_integer_div_branch(x,y)\
    if (guin_is_number(x.current_value_type) && guin_is_number(y.current_value_type)) {\
        new = guin_init_ValueHeader(GINSTRDATATYPE_NUMBER64);\
        guin_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32;\
        guin_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;\
        if (yval == 0)\
            new.n64 = guin_NaN;\
        else\
            new.n64 = xval / yval;\
    }\
    else if (guin_is_number(x.current_value_type)) {\
        new = guin_init_ValueHeader(GINSTRDATATYPE_NUMBER64);\
        guin_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32;\
        guin_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;\
        if (yval == 0)\
            new.n64 = guin_NaN;\
        else\
            new.n64 = xval / yval;\
    }\
    else if (guin_is_number(y.current_value_type)) {\
        new = guin_init_ValueHeader(GINSTRDATATYPE_NUMBER64);\
        guin_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32;\
        guin_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;\
        if (yval == 0)\
            new.n64 = guin_NaN;\
        else\
            new.n64 = xval / yval;\
    }\
    else {\
        new = guin_init_ValueHeader(GINSTRDATATYPE_NUMBER64);\
        guin_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32;\
        guin_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;\
        if (yval == 0)\
            new.n64 = guin_NaN;\
        else\
            new.n64 = xval / yval;\
    }

// ARITHMETIC
bool guin_VM_add(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new;
    // number/int + number/int
    if (guin_is_number_variant(x.current_value_type) && guin_is_number_variant(y.current_value_type)) {
        guin_VM_integer_branch(x,y,
            new.n64 = xval + yval,
                GINSTRDATATYPE_NUMBER64,
            new.n64 = xval + yval,
                GINSTRDATATYPE_NUMBER64,
            new.n64 = xval + yval,
                GINSTRDATATYPE_NUMBER64,
            new.i64 = xval + yval,
                GINSTRDATATYPE_INT64
        )
    }
    // string + string
    else if (x.current_value_type == GINSTRDATATYPE_STRING && y.current_value_type == GINSTRDATATYPE_STRING) {
        guin_stringconcat(x.str, y.str);
        new = x;
        guin_clearstring_ptr(&y.str);
    }
    // error add
    else {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_sub(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new;
    // number/int + number/int
    if (guin_is_number_variant(x.current_value_type) && guin_is_number_variant(y.current_value_type)) {
        guin_VM_integer_branch(x,y,
            new.n64 = xval - yval,
                GINSTRDATATYPE_NUMBER64,
            new.n64 = xval - yval,
                GINSTRDATATYPE_NUMBER64,
            new.n64 = xval - yval,
                GINSTRDATATYPE_NUMBER64,
            new.i64 = xval - yval,
                GINSTRDATATYPE_INT64
        )
    }
    // error sub
    else {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_mul(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new;
    // number/int * number/int
    if (guin_is_number_variant(x.current_value_type) && guin_is_number_variant(y.current_value_type)) {
        guin_VM_integer_branch(x,y,
            new.n64 = xval * yval,
                GINSTRDATATYPE_NUMBER64,
            new.n64 = xval * yval,
                GINSTRDATATYPE_NUMBER64,
            new.n64 = xval * yval,
                GINSTRDATATYPE_NUMBER64,
            new.i64 = xval * yval,
                GINSTRDATATYPE_INT64
        )
    }
    // string/int * string/int
    else if ((x.current_value_type == GINSTRDATATYPE_STRING || y.current_value_type == GINSTRDATATYPE_STRING) && (guin_is_number_variant(x.current_value_type) || guin_is_number_variant(y.current_value_type))) {
        guin_ValueHeader* hd;
        guin_int64 loopcount;

        if (guin_is_number_variant(x.current_value_type)) {
            hd = &y;
            if (guin_is_number(x.current_value_type))            
                loopcount = (guin_int64)((x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32);
            else
                loopcount = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32;
        } else {
            hd = &x;
            loopcount = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
        }

        new = (guin_ValueHeader){.current_value_type=GINSTRDATATYPE_STRING, .str=guin_init_String_ptr()};
        if (new.str->size <= hd->str->length * loopcount) {
            new.str->size = hd->str->length * loopcount + 1;
            char* tmp = realloc(new.str->content, sizeof(char) * new.str->size);
            if (tmp) {
                new.str->content = tmp;
            } else {
                new.str->size = new.str->length / loopcount - 1;
            }
        }
        for (guin_int64 i = 0; i < loopcount; ++i)
            guin_stringconcat(new.str, hd->str);

        guin_destroy_ValueHeader(hd);
    }
    // error mul
    else {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_div(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new;
    // number/int / number/int
    if (guin_is_number_variant(x.current_value_type) && guin_is_number_variant(y.current_value_type)) {
        guin_VM_integer_div_branch(x,y)
    }
    // error div
    else {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_mod(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new;
    // number/int ^ number/int
    if (guin_is_number_variant(x.current_value_type) && guin_is_number_variant(y.current_value_type)) {
        guin_VM_integer_branch(x,y,
            new.n64 = fmod(xval, yval),
                GINSTRDATATYPE_NUMBER64,
            new.n64 = fmod(xval, yval),
                GINSTRDATATYPE_NUMBER64,
            new.n64 = fmod(xval, yval),
                GINSTRDATATYPE_NUMBER64,
            new.n64 = fmod(xval, yval),
                GINSTRDATATYPE_NUMBER64
        )
    }
    // error pow
    else {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_pow(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new;
    // number/int ^ number/int
    if (guin_is_number_variant(x.current_value_type) && guin_is_number_variant(y.current_value_type)) {
        guin_VM_integer_branch(x,y,
            new.n64 = pow(xval, yval),
                GINSTRDATATYPE_NUMBER64,
            new.n64 = pow(xval, yval),
                GINSTRDATATYPE_NUMBER64,
            new.n64 = pow(xval, yval),
                GINSTRDATATYPE_NUMBER64,
            new.n64 = pow(xval, yval),
                GINSTRDATATYPE_NUMBER64
        )
    }
    // error pow
    else {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_neg(guin_VM* vm)
{
    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        return false;
    }

    guin_ValueHeader new = (guin_ValueHeader){0};
    // number/int/bool/char == number/int/bool/char
    if (guin_is_number_variant(x.current_value_type)) {
        if (guin_is_number(x.current_value_type)) {
            guin_number64 xval = (x.current_value_type == GINSTRDATATYPE_NUMBER64)? x.n64 : x.n32; 
            new.n64 = -xval;
            new.current_value_type = GINSTRDATATYPE_NUMBER64;
        }
        else {
            guin_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            new.i64 = -xval;
            new.current_value_type = GINSTRDATATYPE_INT64;
        }
    }
    // error neg
    else {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

// LOGIC
bool guin_VM_not(guin_VM* vm)
{
    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL)
        return false;

    guin_ValueHeader new = (guin_ValueHeader){0,.bl=guin_is_false_eval(x),.current_value_type=GINSTRDATATYPE_BOOL};
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_and(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new = {0, .bl=(!guin_is_false_eval(x) && !guin_is_false_eval(y)), .current_value_type=GINSTRDATATYPE_BOOL};
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_or(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new = {0, .bl=(!guin_is_false_eval(x) || !guin_is_false_eval(y)), .current_value_type=GINSTRDATATYPE_BOOL};
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_equ(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new = {.current_value_type=GINSTRDATATYPE_BOOL,.header_type=GINSTRDATATYPE_BOOL};
    // number/int/bool/char == number/int/bool/char
    if (guin_is_number_variant(x.current_value_type) && guin_is_number_variant(y.current_value_type)) {
        guin_VM_integer_branch(x,y,
            new.bl = xval == yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval == yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval == yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval == yval,
                GINSTRDATATYPE_BOOL
        )
    }
    else if (x.current_value_type == GINSTRDATATYPE_STRING && y.current_value_type == GINSTRDATATYPE_STRING) {
        new.bl = guin_stringcompare(x.str, y.str);
        guin_clearstring_ptr(&x.str);
        guin_clearstring_ptr(&y.str);
    }
    else {
        new.bl = x.current_value_type == y.current_value_type;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_not_equ(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new;
    // number/int/bool/char != number/int/bool/char
    if (guin_is_number_variant(x.current_value_type) && guin_is_number_variant(y.current_value_type)) {
        guin_VM_integer_branch(x,y,
            new.bl = xval != yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval != yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval != yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval != yval,
                GINSTRDATATYPE_BOOL
        )
    }
    else if (x.current_value_type == GINSTRDATATYPE_STRING && y.current_value_type == GINSTRDATATYPE_STRING) {
        new.bl = !guin_stringcompare(x.str, y.str);
        guin_clearstring_ptr(&x.str);
        guin_clearstring_ptr(&y.str);
    }
    else {
        new.bl = x.current_value_type != y.current_value_type;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_gt(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new;
    // number/int/bool/char > number/int/bool/char
    if (guin_is_number_variant(x.current_value_type) && guin_is_number_variant(y.current_value_type)) {
        guin_VM_integer_branch(x,y,
            new.bl = xval > yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval > yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval > yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval > yval,
                GINSTRDATATYPE_BOOL
        )
    }
    else {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_lt(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new;
    // number/int/bool/char < number/int/bool/char
    if (guin_is_number_variant(x.current_value_type) && guin_is_number_variant(y.current_value_type)) {
        guin_VM_integer_branch(x,y,
            new.bl = xval < yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval < yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval < yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval < yval,
                GINSTRDATATYPE_BOOL
        )
    }
    else {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_gt_equ(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new = (guin_ValueHeader){.current_value_type=GINSTRDATATYPE_BOOL};
    // number/int/bool/char >= number/int/bool/char
    if (guin_is_number_variant(x.current_value_type) && guin_is_number_variant(y.current_value_type)) {
        guin_VM_integer_branch(x,y,
            new.bl = xval >= yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval >= yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval >= yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval >= yval,
                GINSTRDATATYPE_BOOL
        )
    }
    else {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}

bool guin_VM_lt_equ(guin_VM* vm)
{
    guin_ValueHeader y = guin_pop_VALUE_STACK(&vm->stack_main);
    if (y.current_value_type == GINSTRDATATYPE_NULL) return false;

    guin_ValueHeader x = guin_pop_VALUE_STACK(&vm->stack_main);
    if (x.current_value_type == GINSTRDATATYPE_NULL) {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }

    guin_ValueHeader new = (guin_ValueHeader){.current_value_type=GINSTRDATATYPE_BOOL};
    // number/int/bool/char <= number/int/bool/char
    if (guin_is_number_variant(x.current_value_type) && guin_is_number_variant(y.current_value_type)) {
        guin_VM_integer_branch(x,y,
            new.bl = xval <= yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval <= yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval <= yval,
                GINSTRDATATYPE_BOOL,
            new.bl = xval <= yval,
                GINSTRDATATYPE_BOOL
        )
    }
    else {
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
        guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, y);
        return false;
    }
    new.header_type = new.current_value_type; // just in case
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, new);
}



guin_STATUS GINSTR_VM_PUSH_IMMEDIATE_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
    ++frame->pc;
    guin_VH_from_BC_result x = guin_get_ValueHeader_from_Bytecode(&frame->func->funcval->bytecode, frame->pc);
    if (x.status == guin_FAIL) {
        guin_printf("%sc\n", x.errmsg);
        guin_destroy_ValueHeader(&x.value);
        return guin_FAIL;
    }
    if (x.status == guin_MEM_FAIL) {
        guin_destroy_ValueHeader(&x.value);
        return guin_MEM_FAIL;
    }

    bool s = guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x.value);
    if (!s) {
        guin_destroy_ValueHeader(&x.value);
        return s;
    }
    frame->pc += x.to_jump;
    return guin_SUCCESS;
}
guin_STATUS GINSTR_VM_DECLARE_GLOBAL_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
    ++frame->pc;
    GINSTR_Datatype dt = *frame->pc;
    ++frame->pc;
    guin_uint64 namelen;
    if (!guin_safe_memcpy_Bytecode(&namelen, &frame->func->funcval->bytecode, frame->pc, sizeof(namelen))) {
        return guin_CORRUPTED_DATA;
    }
    frame->pc += sizeof(namelen);
    char* name = malloc(namelen + 1);
    if (!name) return guin_MEM_FAIL;

    for (size_t i = 0; i < (size_t)namelen; ++i, ++frame->pc)
        name[i] = *frame->pc;
    name[namelen] = 0;

    guin_STATUS s = guin_add_GLOBALNAME_to_GLOBALMAP(&vm->global, dt, name);
    if (s != guin_SUCCESS) {
        free(name);
        return s;
    }

    { // assigns the Global variable its own dedicated slot and assigns from stack
        guin_ValueHeader** x = guin_GC_get_memory(vm, 1);
        if (!x) return guin_MEM_FAIL;
        
        guin_VARIABLE_HEADER* glocation = guin_fetch_GLOBALNAME_from_GLOBALMAP(&vm->global, name);
        glocation->ptr_to_value = *x;
        glocation->ptr_to_value->header_type = dt;
        free(x);

        guin_ValueHeader popped_val = guin_pop_VALUE_STACK(&vm->stack_main);
        s = guin_assign_ValueHeader_with_ValueHeader(glocation->ptr_to_value, popped_val);
        if (s != guin_SUCCESS) {
            printf("unsuccessful assigning to `%s`\n", name);
            if (popped_val.current_value_type != GINSTRDATATYPE_NULL)
                guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, popped_val);
            return s;
        }
    }
    return guin_SUCCESS;
}

#define guin_BASIC_OP_ARITHMETIC_LOGIC(func)\
{\
    if (!func(vm)) return guin_FAIL;\
    ++frame->pc;\
    return guin_SUCCESS;\
}
guin_STATUS GINSTR_VM_ADD_BYTECODE(guin_VM* vm, guin_FRAME* frame)      guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_add)
guin_STATUS GINSTR_VM_SUB_BYTECODE(guin_VM* vm, guin_FRAME* frame)      guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_sub)
guin_STATUS GINSTR_VM_MUL_BYTECODE(guin_VM* vm, guin_FRAME* frame)      guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_mul)
guin_STATUS GINSTR_VM_DIV_BYTECODE(guin_VM* vm, guin_FRAME* frame)      guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_div)
guin_STATUS GINSTR_VM_MOD_BYTECODE(guin_VM* vm, guin_FRAME* frame)      guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_mod)
guin_STATUS GINSTR_VM_POW_BYTECODE(guin_VM* vm, guin_FRAME* frame)      guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_pow)
guin_STATUS GINSTR_VM_NEG_BYTECODE(guin_VM* vm, guin_FRAME* frame)      guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_neg)
guin_STATUS GINSTR_VM_NOT_BYTECODE(guin_VM* vm, guin_FRAME* frame)      guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_not)
guin_STATUS GINSTR_VM_AND_BYTECODE(guin_VM* vm, guin_FRAME* frame)      guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_and)
guin_STATUS GINSTR_VM_OR_BYTECODE(guin_VM* vm, guin_FRAME* frame)       guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_or)
guin_STATUS GINSTR_VM_EQU_BYTECODE(guin_VM* vm, guin_FRAME* frame)      guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_equ)
guin_STATUS GINSTR_VM_NOT_EQU_BYTECODE(guin_VM* vm, guin_FRAME* frame)  guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_not_equ)
guin_STATUS GINSTR_VM_GT_BYTECODE(guin_VM* vm, guin_FRAME* frame)       guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_gt)
guin_STATUS GINSTR_VM_LT_BYTECODE(guin_VM* vm, guin_FRAME* frame)       guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_lt)
guin_STATUS GINSTR_VM_GT_EQU_BYTECODE(guin_VM* vm, guin_FRAME* frame)   guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_gt_equ)
guin_STATUS GINSTR_VM_LT_EQU_BYTECODE(guin_VM* vm, guin_FRAME* frame)   guin_BASIC_OP_ARITHMETIC_LOGIC(guin_VM_lt_equ)

#define guin_VM_is_valid_Program_Counter(pc, bytecode) (pc >= bytecode.bytecode && pc < bytecode.bytecode + bytecode.length)

guin_STATUS GINSTR_VM_JMP_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
    frame->pc += 1;
    guin_ubyte jmp;

    if (!guin_safe_memcpy_Bytecode(&jmp, &frame->func->funcval->bytecode, frame->pc, sizeof(guin_ubyte))) {
        vm->errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
        return guin_FAIL;
    }

    frame->pc += *(frame->pc - 1);
    return guin_SUCCESS;
}
guin_STATUS GINSTR_VM_JMPL_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
    frame->pc += 1;
    guin_JUMPL_SIZE jmp;

    if (!guin_safe_memcpy_Bytecode(&jmp, &frame->func->funcval->bytecode, frame->pc, sizeof(guin_JUMPL_SIZE))) {
        vm->errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
        return guin_FAIL;
    }

    frame->pc += jmp + sizeof(guin_JUMPL_SIZE);
    return guin_SUCCESS;
}

guin_STATUS GINSTR_VM_RET_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
    if (frame->no_ret && frame->func->funcval->return_type != GINSTRDATATYPE_VOID)
        guin_pop_VALUE_STACK(&vm->stack_main);

    guin_pop_FRAME_STACK(&vm->stack_frames);    
    return guin_SUCCESS;
}

typedef guin_STATUS (*guin_BytecodeHandler)(guin_VM* vm, guin_FRAME* frame);
static guin_BytecodeHandler guin_VM_bytecode_handler[256] = {
    [GINSTR_PUSH_IMMEDIATE] = &GINSTR_VM_PUSH_IMMEDIATE_BYTECODE,
    [GINSTR_DECLARE_GLOBAL] = &GINSTR_VM_DECLARE_GLOBAL_BYTECODE,

    [GINSTR_JMP]            = &GINSTR_VM_JMP_BYTECODE,
    [GINSTR_JMPL]           = &GINSTR_VM_JMPL_BYTECODE,

    [GINSTR_RET]            = &GINSTR_VM_RET_BYTECODE,

    [GINSTR_ADD]            = &GINSTR_VM_ADD_BYTECODE,
    [GINSTR_SUB]            = &GINSTR_VM_SUB_BYTECODE,
    [GINSTR_MUL]            = &GINSTR_VM_MUL_BYTECODE,
    [GINSTR_DIV]            = &GINSTR_VM_DIV_BYTECODE,
    [GINSTR_MOD]            = &GINSTR_VM_MOD_BYTECODE,
    [GINSTR_POW]            = &GINSTR_VM_POW_BYTECODE,
    [GINSTR_NEG]            = &GINSTR_VM_NEG_BYTECODE,
    [GINSTR_NOT]            = &GINSTR_VM_NOT_BYTECODE,
    [GINSTR_AND]            = &GINSTR_VM_AND_BYTECODE,
    [GINSTR_OR]             = &GINSTR_VM_OR_BYTECODE,
    [GINSTR_EQU]            = &GINSTR_VM_EQU_BYTECODE,
    [GINSTR_NOT_EQU]        = &GINSTR_VM_NOT_EQU_BYTECODE,
    [GINSTR_GT]             = &GINSTR_VM_GT_BYTECODE,
    [GINSTR_LT]             = &GINSTR_VM_LT_BYTECODE,
    [GINSTR_GT_EQU]         = &GINSTR_VM_GT_EQU_BYTECODE,
    [GINSTR_LT_EQU]         = &GINSTR_VM_LT_EQU_BYTECODE,
};


// DOES NOT DEEP COPY
guin_STATUS guin_load_Bytecode_into_VM(guin_VM* vm, guin_Bytecode* global)
{
    if (!vm || !global) return guin_FAIL;

    guin_FunctionValue* func = malloc(sizeof(guin_FunctionValue));
    func->argc = 0;
    func->bytecode = *global;
    func->return_type = GINSTRDATATYPE_VOID;

    guin_ValueHeader** y = guin_GC_get_memory(vm, 1);
    guin_ValueHeader* x = *y;
    free(y);

    x->current_value_type = GINSTRDATATYPE_FUNCTION;
    x->funcval = func;
    guin_FRAME frame = guin_init_FRAME(x);

    if (!frame.pc)
        return guin_FAIL;

    return (guin_add_FRAME_to_FRAME_STACK(&vm->stack_frames, frame))? guin_SUCCESS : guin_MEM_FAIL;
}


guin_STATUS guin_run_VM(guin_VM* vm)
{
    if (!vm) return guin_FAIL;
    vm->errmsg = NULL;
    for (guin_FRAME* frame = vm->stack_frames.stackptr-1; vm->stack_frames.stackptr != vm->stack_frames.baseptr; frame = vm->stack_frames.stackptr-1) {
        guin_Bytecode* bc = &frame->func->funcval->bytecode;

        if (guin_overflow_Bytecode(bc, frame->pc)) {
            vm->errmsg = "Program Counter overflow!";
            return guin_FAIL;
        }
            
        if (guin_underflow_Bytecode(bc, frame->pc)) {
            vm->errmsg = "Program Counter underflow!";
            return guin_FAIL;
        }
            
        guin_BytecodeHandler func = guin_VM_bytecode_handler[*frame->pc];
        if (func == NULL) {
            vm->errmsg = "Unknown bytecode!";
            //printf("Error byte: %d %zu\n", *frame->pc, (size_t)(frame->pc - frame->func->funcval->bytecode.bytecode));
            return guin_FAIL;
        }
        guin_STATUS s = func(vm, frame);
        if (s != guin_SUCCESS) return s;
    }
    return guin_SUCCESS;
}

// true:  loaded and did execute (but not necessarily there was no runtime errors)
// false: failed to load/execute
guin_STATUS guin_exec_VM(guin_VM* vm, char* start)
{
    guin_VARIABLE_HEADER* start_header = guin_fetch_GLOBALNAME_from_GLOBALMAP(&vm->global, start);
    if (!start_header) return guin_FAIL;
    if (start_header->datatype != GINSTRDATATYPE_FUNCTION || !start_header->ptr_to_value) return guin_FAIL;

    return guin_SUCCESS;
}

#endif
