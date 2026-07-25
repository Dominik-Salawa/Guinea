#ifndef GUIN_VM_H
#define GUIN_VM_H

#include <stdbool.h>
#include "../bytecode.h"
#include "stacks.h"
#include "value.h"

#define GUIN_is_false_eval(x) (x.current_value_type == GINSTRDATATYPE_NIL || (x.current_value_type == GINSTRDATATYPE_BOOL && !x.bl))

typedef struct GUIN_VM {
    GUIN_FRAME_STACK  stack_frames;
    GUIN_VALUE_STACK        stack_main;
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

// true:  loaded and did execute (but not necessarily there was no runtime errors)
// false: failed to load/execute
bool GUIN_exec_VM(GUIN_VM* gruntime);

#endif
