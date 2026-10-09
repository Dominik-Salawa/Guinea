#ifndef guin_VALUE_h
#define guin_VALUE_h

#include <inttypes.h>
#include <stdlib.h>
#include <stdalign.h>
#include "../../etc/strings.h"
#include "../../../include/declarations.h"
#include "../bytecode.h"

typedef struct guin_FunctionValue {
    guin_ubyte     return_type; // GINSTR_Datatype
    guin_uint16    argc;
    guin_Bytecode  bytecode;
} guin_FunctionValue;

typedef struct guin_ValueHeader {
    guin_ubyte header_type; // GINSTR_Datatype
    guin_ubyte current_value_type;  // GINSTR_Datatype
    guin_ubyte GC_status;
    guin_ubyte struct_value_type;
    guin_uint32 children_count;
    union {
        bool                    bl;
        char                    ch;
        guin_number32           n32;
        guin_number64           n64;
        guin_int32              i32;
        guin_int64              i64;
        guin_String*            str;
        guin_FunctionValue*     funcval;
        struct guin_ValueHeader** children;
    };
} guin_ValueHeader;
guin_ValueHeader guin_init_ValueHeader(GINSTR_Datatype header_type);
void guin_destroy_ValueHeader(guin_ValueHeader* x);


typedef struct guin_VH_from_BC_result {
    guin_ValueHeader value;
    size_t to_jump;
    guin_STATUS status;
    char* errmsg;
} guin_VH_from_BC_result;
// make sure source starts on the datatype
guin_VH_from_BC_result guin_get_ValueHeader_from_Bytecode(guin_Bytecode* bytecode, guin_ubyte* src);

#endif
