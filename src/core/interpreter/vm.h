#ifndef guin_VM_h
#define guin_VM_h

#include <stdbool.h>
#include "../../../include/declarations.h"
#include "../bytecode.h"
#include "stacks.h"
#include "value.h"
#include "gc.h"

#define guin_is_false_eval(x) (x.current_value_type == GINSTRDATATYPE_NIL || (x.current_value_type == GINSTRDATATYPE_BOOL && !x.bl))

typedef struct guin_GLOBALMAP {
    size_t size;
    size_t length;
    guin_VARIABLE_HEADER* content;
} guin_GLOBALMAP;
guin_GLOBALMAP guin_init_GLOBALMAP(void);
void guin_destroy_GLOBALMAP(guin_GLOBALMAP* x);
guin_STATUS guin_add_GLOBALNAME_to_GLOBALMAP(guin_GLOBALMAP* x, GINSTR_Datatype datatype, char* global_name);
guin_VARIABLE_HEADER* guin_fetch_GLOBALNAME_from_GLOBALMAP(guin_GLOBALMAP* x, char* global_name);

typedef struct guin_VM {
    guin_FRAME_STACK  stack_frames;
    guin_VALUE_STACK  stack_main;
    guin_GLOBALMAP    global;
    guin_GC           gc;
    guin_ubyte*       pc;
	guin_ValueHeader* load;
    char* errmsg;
} guin_VM;
guin_VM  guin_init_VM(void);
guin_VM* guin_init_VM_ptr(void);
void guin_destroy_VM(guin_VM* x);
void guin_destroy_VM_ptr(guin_VM** x);

bool guin_VM_add(guin_VM* vm);
bool guin_VM_sub(guin_VM* vm);
bool guin_VM_mul(guin_VM* vm);
bool guin_VM_div(guin_VM* vm);
bool guin_VM_mod(guin_VM* vm);
bool guin_VM_pow(guin_VM* vm);

bool guin_VM_not(guin_VM* vm);
bool guin_VM_and(guin_VM* vm);
bool guin_VM_or(guin_VM* vm);
bool guin_VM_equ(guin_VM* vm);
bool guin_VM_not_equ(guin_VM* vm);
bool guin_VM_lt(guin_VM* vm);
bool guin_VM_gt(guin_VM* vm);
bool guin_VM_lt_equ(guin_VM* vm);
bool guin_VM_gt_equ(guin_VM* vm);

// does NOT deep copy
guin_STATUS guin_load_Bytecode_into_VM(guin_VM* vm, guin_Bytecode* bytecode);
guin_STATUS guin_run_VM(guin_VM* vm);
// true:  loaded and did execute (but not necessarily there was no runtime errors)
// false: failed to load/execute
guin_STATUS guin_exec_VM(guin_VM* vm, char* _start);

#endif
