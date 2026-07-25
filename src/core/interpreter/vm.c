#ifndef GUIN_VM_C
#define GUIN_VM_C

#include "stacks.h"
#include "vm.h"
#include <math.h>

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
    printf("destroying VM\n");
    GUIN_destroy_VALUE_STACK(&x->stack_main);
    printf("freed main stack\n");
    GUIN_destroy_FRAME_STACK(&x->stack_frames);
    printf("freed frame stack\n");
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
            GUIN_int64 yval    = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
            new.n64 = xval / yval;
        }
        else if (GUIN_is_number(y.current_value_type)) {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_int64 xval    = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_number64 yval = (y.current_value_type == GINSTRDATATYPE_NUMBER64)? y.n64 : y.n32;
            new.n64 = xval / yval;
        }
        else {
            new = (GUIN_ValueHeader){.current_value_type=GINSTRDATATYPE_NUMBER64};
            GUIN_int64 xval = (x.current_value_type == GINSTRDATATYPE_INT64)? x.i64 : x.i32; 
            GUIN_int64 yval = (y.current_value_type == GINSTRDATATYPE_INT64)? y.i64 : y.i32;
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
    else if (x.current_value_type != y.current_value_type) {
        new.bl = false;
    }
    else if (x.current_value_type == GINSTRDATATYPE_STRING && y.current_value_type == GINSTRDATATYPE_STRING) {
        new.bl = GUIN_stringcompare(x.str, y.str);
        GUIN_clearstring_ptr(&x.str);
        GUIN_clearstring_ptr(&y.str);
    }
    else {
        new.bl = true;
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
    else if (x.current_value_type != y.current_value_type) {
        new.bl = true;
    }
    else if (x.current_value_type == GINSTRDATATYPE_STRING && y.current_value_type == GINSTRDATATYPE_STRING) {
        new.bl = !GUIN_stringcompare(x.str, y.str);
        GUIN_clearstring_ptr(&x.str);
        GUIN_clearstring_ptr(&y.str);
    }
    else {
        new.bl = false;
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

// true:  loaded and did execute (but not necessarily there was no runtime errors)
// false: failed to load/execute
bool GUIN_exec_VM(GUIN_VM* vm);

#endif
