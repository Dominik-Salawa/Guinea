#ifndef guin_STACKS_c
#define guin_STACKS_c

#include <stdlib.h>
#include <string.h>
#include "stacks.h"

guin_VARIABLE_HEADER  guin_init_VARIABLE_HEADER(GINSTR_Datatype datatype, char* name, guin_ValueHeader* ptr)
{
    guin_VARIABLE_HEADER x = (guin_VARIABLE_HEADER){0};
    if (!datatype) return x;

    x.datatype = datatype;
    x.name = name;
    x.ptr_to_value = ptr;
    return x;
}
guin_VARIABLE_HEADER  guin_init_VARIABLE_HEADER_cp_name(GINSTR_Datatype datatype, char* name, guin_ValueHeader* ptr)
{
    guin_VARIABLE_HEADER x = (guin_VARIABLE_HEADER){0};
    if (!datatype) return x;

    x.datatype = datatype;
    x.name = malloc(sizeof(char) * strlen(name) + 1);
    if (!x.name) {
        return (guin_VARIABLE_HEADER){0};
    }
    strcpy(x.name, name);
    x.ptr_to_value = ptr;
    return x;
}
guin_VARIABLE_HEADER* guin_init_VARIABLE_HEADER_ptr(GINSTR_Datatype datatype, char* name, guin_ValueHeader* ptr)
{
    if (!datatype) return NULL;

    guin_VARIABLE_HEADER* x = malloc(sizeof(guin_VARIABLE_HEADER));
    if (!x) return NULL;

    *x = guin_init_VARIABLE_HEADER(datatype, name, ptr);
    if (!x->datatype) {
        if (x->name) free(x->name);
        free(x);
        return NULL;
    }

    return x;
}
void guin_destroy_VARIABLE_HEADER(guin_VARIABLE_HEADER* x)
{
    if (x->name) free(x->name);
    *x = (guin_VARIABLE_HEADER){0};
}
void guin_destroy_VARIABLE_HEADER_ptr(guin_VARIABLE_HEADER** x)
{
    if (!x)  return;
    if (!*x) return;
    if ((*x)->name) free((*x)->name);
    free(*x);
    *x = NULL;
}





guin_VALUE_STACK guin_init_VALUE_STACK(void)
{
    guin_VALUE_STACK x = {0};
    x.size = guin_STACK_INITIAL_SIZE;
    x.baseptr = malloc(x.size * sizeof(guin_ValueHeader));
    x.stackptr = x.baseptr;

    if (!x.baseptr)
        x = (guin_VALUE_STACK){0};

    return x;
}
void guin_destroy_VALUE_STACK(guin_VALUE_STACK* x)
{
    if (!x) return;
    if (x->baseptr) {
        for (guin_ValueHeader* ptr = x->baseptr; ptr < x->stackptr; ++ptr)
            guin_destroy_ValueHeader(ptr);
        free(x->baseptr);
    }
    *x = (guin_VALUE_STACK){0}; // nullifies everything
}
bool guin_grow_VALUE_STACK(guin_VALUE_STACK* x)
{
    size_t difference_between_base_n_stack = x->stackptr - x->baseptr;
    x->size *= 2;
    guin_ValueHeader* tmp = realloc(x->baseptr, x->size * sizeof(guin_ValueHeader));
    if (!tmp) {
        x->size /= 2;
        return false;
    }
    x->baseptr  = tmp;
    x->stackptr = x->baseptr + difference_between_base_n_stack;
    return true;
}
bool guin_add_VALUE_to_VALUE_STACK(guin_VALUE_STACK* x, guin_ValueHeader val)
{
    if (!x) return false;
    size_t difference_between_base_n_stack = x->stackptr - x->baseptr;
    if (difference_between_base_n_stack >= x->size) {
        if (!guin_grow_VALUE_STACK(x)) {
            return false;
        }
    }
    *x->stackptr++ = val;
    return true; 
}
guin_ValueHeader guin_pop_VALUE_STACK(guin_VALUE_STACK* x)
{
    if (x->stackptr != x->baseptr) {
        --x->stackptr;
        return *x->stackptr;
    }
    return (guin_ValueHeader){0};
}


// will assume that the entire size of local slot is in func
// func is meant to be a reference for the GC, it is under
// risk of segfault if func gets deleted later on through 
// runtime, rejects if func ISNT PTR TO FUNCVAL OBJ
guin_FRAME guin_init_FRAME(guin_ValueHeader* func)
{
    if (!func) return (guin_FRAME){0};
    if (func->current_value_type != GINSTRDATATYPE_FUNCTION) return (guin_FRAME){0};
    guin_FRAME x = (guin_FRAME){0};
    x.func = func;
    x.pc = func->funcval->bytecode.bytecode; // 1st val of bytecode
    return x;
}
// shouldnt destroy or free, as everything is just references
void guin_destroy_FRAME(guin_FRAME* x)
{
    x->func = NULL;
    //x->local_slot_base = 0;
    x->pc = NULL;
}


guin_FRAME_STACK guin_init_FRAME_STACK(void)
{
    guin_FRAME_STACK x = (guin_FRAME_STACK){0};
    x.size = 32;
    x.baseptr = malloc(x.size * sizeof(guin_FRAME));
    if (!x.baseptr)
        return (guin_FRAME_STACK){0};
    x.stackptr = x.baseptr;
    return x;
}
void guin_destroy_FRAME_STACK(guin_FRAME_STACK* x)
{
    if (!x) return;
    if (x->baseptr) {
        for (guin_FRAME* i = x->baseptr; i < x->stackptr; ++i)
            guin_destroy_FRAME(i);
        free(x->baseptr);
    }
    *x = (guin_FRAME_STACK){0};
}
bool guin_grow_FRAME_STACK(guin_FRAME_STACK* x)
{
    size_t difference_between_base_n_stack = x->stackptr - x->baseptr;
    x->size *= 2;
    guin_FRAME* tmp = realloc(x->baseptr, sizeof(guin_FRAME) * x->size);
    if (!tmp) {
        x->size /= 2;
        return false;
    }
    x->baseptr  = tmp;
    x->stackptr = x->baseptr + difference_between_base_n_stack;
    return true;
}
bool guin_add_FRAME_to_FRAME_STACK(guin_FRAME_STACK* x, guin_FRAME frame)
{
    if (!x) return false;
    size_t difference_between_base_n_stack = x->stackptr - x->baseptr;
    if (difference_between_base_n_stack >= x->size) {
        if (!guin_grow_FRAME_STACK(x)) {
            --x->stackptr;
            return false;
        }
    }
    *x->stackptr++ = frame;
    return true;
}
void guin_pop_FRAME_STACK(guin_FRAME_STACK* x)
{
    if (x->stackptr != x->baseptr) --x->stackptr;
}

#endif
