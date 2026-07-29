#ifndef GUIN_VM_H
#define GUIN_VM_H

#include <stdbool.h>
#include "../../../include/declarations.h"
#include "../bytecode.h"
#include "stacks.h"
#include "value.h"

#define GUIN_is_false_eval(x) (x.current_value_type == GINSTRDATATYPE_NIL || (x.current_value_type == GINSTRDATATYPE_BOOL && !x.bl))

typedef struct GUIN_GLOBALMAP {
    size_t size;
    size_t length;
    GUIN_VARIABLE_HEADER* content;
} GUIN_GLOBALMAP;
GUIN_GLOBALMAP GUIN_init_GLOBALMAP(void);
void GUIN_destroy_GLOBALMAP(GUIN_GLOBALMAP* x);
GUIN_STATUS GUIN_add_GLOBALNAME_to_GLOBALMAP(GUIN_GLOBALMAP* x, GINSTR_Datatype datatype, char* global_name);
GUIN_VARIABLE_HEADER* GUIN_fetch_GLOBALNAME_from_GLOBALMAP(GUIN_GLOBALMAP* x, char* global_name);

typedef struct GUIN_VM {
    GUIN_FRAME_STACK stack_frames;
    GUIN_VALUE_STACK stack_main;
    GUIN_GLOBALMAP   global;
} GUIN_VM;
GUIN_VM  GUIN_init_VM(void);
GUIN_VM* GUIN_init_VM_ptr(void);
void GUIN_destroy_VM(GUIN_VM* x);
void GUIN_destroy_VM_ptr(GUIN_VM** x);

bool GUIN_VM_add(GUIN_VM* vm);
bool GUIN_VM_sub(GUIN_VM* vm);
bool GUIN_VM_mul(GUIN_VM* vm);
bool GUIN_VM_div(GUIN_VM* vm);
bool GUIN_VM_mod(GUIN_VM* vm);
bool GUIN_VM_pow(GUIN_VM* vm);

bool GUIN_VM_not(GUIN_VM* vm);
bool GUIN_VM_and(GUIN_VM* vm);
bool GUIN_VM_or(GUIN_VM* vm);
bool GUIN_VM_equ(GUIN_VM* vm);
bool GUIN_VM_not_equ(GUIN_VM* vm);
bool GUIN_VM_lt(GUIN_VM* vm);
bool GUIN_VM_gt(GUIN_VM* vm);
bool GUIN_VM_lt_equ(GUIN_VM* vm);
bool GUIN_VM_gt_equ(GUIN_VM* vm);

GUIN_STATUS GUIN_load_VM(GUIN_VM* vm, GUIN_Bytecode* bytecode);
// true:  loaded and did execute (but not necessarily there was no runtime errors)
// false: failed to load/execute
GUIN_STATUS GUIN_exec_VM(GUIN_VM* vm, char* _start);

#endif
