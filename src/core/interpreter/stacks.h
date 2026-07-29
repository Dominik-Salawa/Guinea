#ifndef GUIN_STACKS_H
#define GUIN_STACKS_H

#include <stdbool.h>
#include "../../../include/declarations.h"
#include "value.h"
#include "../bytecode.h"

const size_t GUIN_STACK_INITIAL_SIZE = 4096;
const size_t GUIN_FRAME_STACK_INITIAL_SIZE = 4096;


typedef struct GUIN_VARIABLE_HEADER {
    GINSTR_Datatype datatype;
    char* name;
    GUIN_ValueHeader* ptr_to_value;
} GUIN_VARIABLE_HEADER;
GUIN_VARIABLE_HEADER  GUIN_init_VARIABLE_HEADER(GINSTR_Datatype datatype, char* name, GUIN_ValueHeader* ptr);
GUIN_VARIABLE_HEADER* GUIN_init_VARIABLE_HEADER_ptr(GINSTR_Datatype datatype, char* name, GUIN_ValueHeader* ptr);
void GUIN_destroy_VARIABLE_HEADER(GUIN_VARIABLE_HEADER* x);
void GUIN_destroy_VARIABLE_HEADER_ptr(GUIN_VARIABLE_HEADER** x);


typedef struct GUIN_VALUE_STACK {
    GUIN_ValueHeader* baseptr;
    GUIN_ValueHeader* stackptr;
    size_t size;
} GUIN_VALUE_STACK;
GUIN_VALUE_STACK GUIN_init_VALUE_STACK(void);
void GUIN_destroy_VALUE_STACK(GUIN_VALUE_STACK* x);
bool GUIN_grow_VALUE_STACK(GUIN_VALUE_STACK* x);
bool GUIN_add_VALUE_to_VALUE_STACK(GUIN_VALUE_STACK* x, GUIN_ValueHeader val);
GUIN_ValueHeader GUIN_pop_VALUE_STACK(GUIN_VALUE_STACK* x);


typedef struct GUIN_FRAME {
    GUIN_ValueHeader* func; // for the GC to not delete it, should trust its pointing to a function obj as well
    size_t local_slot_base; // the offset in the main stack where the first arg starts
    GUIN_ubyte* pc; // points to func->funcval->bytecode and one of its members
} GUIN_FRAME;
// will assume that the entire size of local slot is in func
// func is meant to be a reference for the GC, it is under
// risk of segfault if func gets deleted later on through 
// runtime
GUIN_FRAME GUIN_init_FRAME(GUIN_ValueHeader* func);
// shouldnt destroy or free, as everything is just references
void GUIN_destroy_FRAME(GUIN_FRAME* x);



typedef struct GUIN_FRAME_STACK {
    GUIN_FRAME* baseptr;
    GUIN_FRAME* stackptr;
    size_t size;
} GUIN_FRAME_STACK;
GUIN_FRAME_STACK GUIN_init_FRAME_STACK(void);
void GUIN_destroy_FRAME_STACK(GUIN_FRAME_STACK* x);
bool GUIN_grow_FRAME_STACK(GUIN_FRAME_STACK* x);
// increment the size by the size of GUIN_STACK_INCREMENT_SIZE
bool GUIN_add_FRAME_to_FRAME_STACK(GUIN_FRAME_STACK* x, GUIN_FRAME frame);
void GUIN_pop_FRAME_STACK(GUIN_FRAME_STACK* x);

#endif
