#ifndef GUIN_VALUE_H
#define GUIN_VALUE_H

#include <inttypes.h>
#include <stdlib.h>
#include <stdalign.h>
#include "../../etc/strings.h"
#include "../../../include/declarations.h"
#include "../bytecode.h"

typedef struct GUIN_FunctionValue {
    GUIN_ubyte     return_type; // GINSTR_Datatype
    GUIN_uint16    argc;
    GUIN_Bytecode  bytecode;
} GUIN_FunctionValue;

typedef struct GUIN_ValueHeader {
    GUIN_ubyte header_type; // GINSTR_Datatype
    GUIN_ubyte current_value_type;  // GINSTR_Datatype
    GUIN_ubyte GC_status;
    GUIN_ubyte struct_value_type;
    GUIN_uint32 children_count;
    union {
        bool                    bl;
        char                    ch;
        GUIN_number32           n32;
        GUIN_number64           n64;
        GUIN_int32              i32;
        GUIN_int64              i64;
        GUIN_String*            str;
        GUIN_FunctionValue*     funcval;
        struct GUIN_ValueHeader** children;
    };
} GUIN_ValueHeader;
GUIN_ValueHeader GUIN_init_ValueHeader(GINSTR_Datatype header_type);
void GUIN_destroy_ValueHeader(GUIN_ValueHeader* x);


typedef struct GUIN_VH_from_BC_result {
    GUIN_ValueHeader value;
    size_t to_jump;
    GUIN_STATUS status;
    char* errmsg;
} GUIN_VH_from_BC_result;
// make sure source starts on the datatype
GUIN_VH_from_BC_result GUIN_get_ValueHeader_from_Bytecode(GUIN_ubyte* src);

#endif
