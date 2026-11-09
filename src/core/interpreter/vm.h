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
    guin_ubyte*       pc;
	guin_ValueHeader* load;
    char* errmsg;

	struct guin_Registers {
		struct {
			guin_int64 a;
			guin_int64 b;
			guin_int64 c;
			guin_int64 d;
			guin_int64 e;
		} i;

		struct {
			guin_number64 a;
			guin_number64 b;
			guin_number64 c;
			guin_number64 d;
			guin_number64 e;
		} n;
	} reg;
} guin_VM;
guin_VM  guin_init_VM(void);
guin_VM* guin_init_VM_ptr(void);
void guin_destroy_VM(guin_VM* x);
void guin_destroy_VM_ptr(guin_VM** x);

#endif
