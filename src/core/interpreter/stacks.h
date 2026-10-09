#ifndef guin_STACKS_h
#define guin_STACKS_h

#include <stdbool.h>
#include "../../../include/declarations.h"
#include "value.h"
#include "../bytecode.h"

const size_t guin_STACK_INITIAL_SIZE = 4096;
const size_t guin_FRAME_STACK_INITIAL_SIZE = 4096;


typedef struct guin_VARIABLE_HEADER {
    GINSTR_Datatype datatype;
    char* name;
    guin_ValueHeader* ptr_to_value;
} guin_VARIABLE_HEADER;
guin_VARIABLE_HEADER  guin_init_VARIABLE_HEADER(GINSTR_Datatype datatype, char* name, guin_ValueHeader* ptr);
guin_VARIABLE_HEADER  guin_init_VARIABLE_HEADER_cp_name(GINSTR_Datatype datatype, char* name, guin_ValueHeader* ptr);
guin_VARIABLE_HEADER* guin_init_VARIABLE_HEADER_ptr(GINSTR_Datatype datatype, char* name, guin_ValueHeader* ptr);
void guin_destroy_VARIABLE_HEADER(guin_VARIABLE_HEADER* x);
void guin_destroy_VARIABLE_HEADER_ptr(guin_VARIABLE_HEADER** x);


typedef struct guin_VALUE_STACK {
    guin_ValueHeader* baseptr;
    guin_ValueHeader* stackptr;
    size_t size;
} guin_VALUE_STACK;
guin_VALUE_STACK guin_init_VALUE_STACK(void);
void guin_destroy_VALUE_STACK(guin_VALUE_STACK* x);
bool guin_grow_VALUE_STACK(guin_VALUE_STACK* x);
bool guin_add_VALUE_to_VALUE_STACK(guin_VALUE_STACK* x, guin_ValueHeader val);
guin_ValueHeader guin_pop_VALUE_STACK(guin_VALUE_STACK* x);


typedef struct guin_FRAME {
    guin_ValueHeader* func; // for the GC to not delete it, should trust its pointing to a function obj as well
    size_t local_slot_base; // the offset in the main stack where the first arg starts
    guin_ubyte* pc; // points to func->funcval->bytecode and one of its members
    bool no_ret;
} guin_FRAME;
// will assume that the entire size of local slot is in func
// func is meant to be a reference for the GC, it is under
// risk of segfault if func gets deleted later on through 
// runtime
guin_FRAME guin_init_FRAME(guin_ValueHeader* func);
// shouldnt destroy or free, as everything is just references
void guin_destroy_FRAME(guin_FRAME* x);



typedef struct guin_FRAME_STACK {
    guin_FRAME* baseptr;
    guin_FRAME* stackptr;
    size_t size;
} guin_FRAME_STACK;
guin_FRAME_STACK guin_init_FRAME_STACK(void);
void guin_destroy_FRAME_STACK(guin_FRAME_STACK* x);
bool guin_grow_FRAME_STACK(guin_FRAME_STACK* x);
// increment the size by the size of guin_STACK_INCREMENT_SIZE
bool guin_add_FRAME_to_FRAME_STACK(guin_FRAME_STACK* x, guin_FRAME frame);
void guin_pop_FRAME_STACK(guin_FRAME_STACK* x);

#endif
