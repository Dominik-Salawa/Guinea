#ifndef GUIN_STACKS_C
#define GUIN_STACKS_C

#include <stdlib.h>
#include <string.h>
#include "stacks.h"

GUIN_VARIABLE_HEADER  GUIN_init_VARIABLE_HEADER(GINSTR_Datatype datatype, char* name, GUIN_ValueHeader* ptr)
{
    GUIN_VARIABLE_HEADER x = (GUIN_VARIABLE_HEADER){0};
    if (!datatype) return x;

    x.datatype = datatype;
    x.name = name;
    x.ptr_to_value = ptr;
    return x;
}
GUIN_VARIABLE_HEADER  GUIN_init_VARIABLE_HEADER_cp_name(GINSTR_Datatype datatype, char* name, GUIN_ValueHeader* ptr)
{
    GUIN_VARIABLE_HEADER x = (GUIN_VARIABLE_HEADER){0};
    if (!datatype) return x;

    x.datatype = datatype;
    x.name = malloc(sizeof(char) * strlen(name) + 1);
    if (!x.name) {
        return (GUIN_VARIABLE_HEADER){0};
    }
    strcpy(x.name, name);
    x.ptr_to_value = ptr;
    return x;
}
GUIN_VARIABLE_HEADER* GUIN_init_VARIABLE_HEADER_ptr(GINSTR_Datatype datatype, char* name, GUIN_ValueHeader* ptr)
{
    if (!datatype) return NULL;

    GUIN_VARIABLE_HEADER* x = malloc(sizeof(GUIN_VARIABLE_HEADER));
    if (!x) return NULL;

    *x = GUIN_init_VARIABLE_HEADER(datatype, name, ptr);
    if (!x->datatype) {
        if (x->name) free(x->name);
        free(x);
        return NULL;
    }

    return x;
}
void GUIN_destroy_VARIABLE_HEADER(GUIN_VARIABLE_HEADER* x)
{
    if (x->name) free(x->name);
    *x = (GUIN_VARIABLE_HEADER){0};
}
void GUIN_destroy_VARIABLE_HEADER_ptr(GUIN_VARIABLE_HEADER** x)
{
    if (!x)  return;
    if (!*x) return;
    if ((*x)->name) free((*x)->name);
    free(*x);
    *x = NULL;
}





GUIN_VALUE_STACK GUIN_init_VALUE_STACK(void)
{
    GUIN_VALUE_STACK x = {0};
    x.size = GUIN_STACK_INITIAL_SIZE;
    x.baseptr = malloc(x.size * sizeof(GUIN_ValueHeader));
    x.stackptr = x.baseptr;

    if (!x.baseptr)
        x = (GUIN_VALUE_STACK){0};

    return x;
}
void GUIN_destroy_VALUE_STACK(GUIN_VALUE_STACK* x)
{
    if (!x) return;
    if (x->baseptr) {
        for (GUIN_ValueHeader* ptr = x->baseptr; ptr < x->stackptr; ++ptr)
            GUIN_destroy_ValueHeader(ptr);
        free(x->baseptr);
    }
    *x = (GUIN_VALUE_STACK){0}; // nullifies everything
}
bool GUIN_grow_VALUE_STACK(GUIN_VALUE_STACK* x)
{
    size_t difference_between_base_n_stack = x->stackptr - x->baseptr;
    x->size *= 2;
    GUIN_ValueHeader* tmp = realloc(x->baseptr, x->size * sizeof(GUIN_ValueHeader));
    if (!tmp) {
        x->size /= 2;
        return false;
    }
    x->baseptr  = tmp;
    x->stackptr = x->baseptr + difference_between_base_n_stack;
    return true;
}
bool GUIN_add_VALUE_to_VALUE_STACK(GUIN_VALUE_STACK* x, GUIN_ValueHeader val)
{
    if (!x) return false;
    size_t difference_between_base_n_stack = x->stackptr - x->baseptr;
    if (difference_between_base_n_stack >= x->size) {
        if (!GUIN_grow_VALUE_STACK(x)) {
            return false;
        }
    }
    *x->stackptr++ = val;
    return true; 
}
GUIN_ValueHeader GUIN_pop_VALUE_STACK(GUIN_VALUE_STACK* x)
{
    if (x->stackptr != x->baseptr) {
        --x->stackptr;
        return *x->stackptr;
    }
    return (GUIN_ValueHeader){0};
}


// will assume that the entire size of local slot is in func
// func is meant to be a reference for the GC, it is under
// risk of segfault if func gets deleted later on through 
// runtime, rejects if func ISNT PTR TO FUNCVAL OBJ
GUIN_FRAME GUIN_init_FRAME(GUIN_ValueHeader* func)
{
    if (!func) return (GUIN_FRAME){0};
    if (func->current_value_type != GINSTRDATATYPE_FUNCTION) return (GUIN_FRAME){0};
    GUIN_FRAME x = (GUIN_FRAME){0};
    x.func = func;
    x.pc = func->funcval->bytecode.bytecode; // 1st val of bytecode
    return x;
}
// shouldnt destroy or free, as everything is just references
void GUIN_destroy_FRAME(GUIN_FRAME* x)
{
    x->func = NULL;
    //x->local_slot_base = 0;
    x->pc = NULL;
}


GUIN_FRAME_STACK GUIN_init_FRAME_STACK(void)
{
    GUIN_FRAME_STACK x = (GUIN_FRAME_STACK){0};
    x.size = 32;
    x.baseptr = malloc(x.size * sizeof(GUIN_FRAME));
    if (!x.baseptr)
        return (GUIN_FRAME_STACK){0};
    x.stackptr = x.baseptr;
    return x;
}
void GUIN_destroy_FRAME_STACK(GUIN_FRAME_STACK* x)
{
    if (!x) return;
    if (x->baseptr) {
        for (GUIN_FRAME* i = x->baseptr; i < x->stackptr; ++i)
            GUIN_destroy_FRAME(i);
        free(x->baseptr);
    }
    *x = (GUIN_FRAME_STACK){0};
}
bool GUIN_grow_FRAME_STACK(GUIN_FRAME_STACK* x)
{
    size_t difference_between_base_n_stack = x->stackptr - x->baseptr;
    x->size *= 2;
    GUIN_FRAME* tmp = realloc(x->baseptr, sizeof(GUIN_FRAME) * x->size);
    if (!tmp) {
        x->size /= 2;
        return false;
    }
    x->baseptr  = tmp;
    x->stackptr = x->baseptr + difference_between_base_n_stack;
    return true;
}
bool GUIN_add_FRAME_to_FRAME_STACK(GUIN_FRAME_STACK* x, GUIN_FRAME frame)
{
    if (!x) return false;
    size_t difference_between_base_n_stack = x->stackptr - x->baseptr;
    if (difference_between_base_n_stack >= x->size) {
        if (!GUIN_grow_FRAME_STACK(x)) {
            --x->stackptr;
            return false;
        }
    }
    *x->stackptr++ = frame;
    return true;
}
void GUIN_pop_FRAME_STACK(GUIN_FRAME_STACK* x)
{
    if (x->stackptr != x->baseptr) --x->stackptr;
}

#endif
