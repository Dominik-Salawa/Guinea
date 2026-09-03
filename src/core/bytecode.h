#ifndef BYTECODE_H
#define BYTECODE_H

#include "../../include/declarations.h"
#include "../etc/strings.h"
#include <stdlib.h>
#include <stdbool.h>

typedef enum GINSTR {
    GINSTR_NULL = 0,
    GINSTR_DECLARE_GLOBAL = 0x01, // tells the interpreter to pop one from the stack, make a new symbol name in the runtime with it, and assign it the stack val
    GINSTR_DECLARE_LOCAL,
    GINSTR_CLEAR_LOCAL,

    // in order to change the value we are pointing to
    GINSTR_LOAD_GLOBAL,
    GINSTR_LOAD_LOCAL,
    GINSTR_LOAD_INDEX,
    GINSTR_LOAD_FIELD,
    GINSTR_WRITE_LOAD,

    // pushing values
    GINSTR_PUSH_LOAD,
    GINSTR_PUSH_IMMEDIATE,

    // operations
    GINSTR_ADD = 0x30,
    GINSTR_SUB,
    GINSTR_MUL,
    GINSTR_DIV,
    GINSTR_MOD,
    GINSTR_POW,

    GINSTR_NOT,
    GINSTR_NEG,
    GINSTR_CALL, // special behavior: itll stop at a certain point, as before the call gets answered the interpreter pushes an EOA (end of args) point, so itll consume all args until that point

    GINSTR_AND,
    GINSTR_OR,

    GINSTR_EQU,
    GINSTR_NOT_EQU,
    GINSTR_GT,
    GINSTR_LT,
    GINSTR_GT_EQU,
    GINSTR_LT_EQU,

    // control flow
    GINSTR_JMP = 0x60, // Jump
    GINSTR_JMPL,       // Jump LONG ( >(2 signed bytes) away )
    GINSTR_JNT,        // Jump Not True
    GINSTR_JNTL,       // Jump Not True LONG ( >(2 signed bytes) away )
    GINSTR_JIT,        // Jump If True
    GINSTR_JITL,       // Jump If True LONG ( >(2 signed bytes) away )
    GINSTR_RET,
} GINSTR;

typedef enum GINSTR_Datatype {
    GINSTRDATATYPE_NULL = 0,
    GINSTRDATATYPE_VOID,
    GINSTRDATATYPE_NIL,
    GINSTRDATATYPE_STRING,
    GINSTRDATATYPE_INT32,
    GINSTRDATATYPE_INT64,
    GINSTRDATATYPE_NUMBER32,
    GINSTRDATATYPE_NUMBER64,
    GINSTRDATATYPE_BOOL,
    GINSTRDATATYPE_CHAR,
    GINSTRDATATYPE_FUNCTION,
    GINSTRDATATYPE_DYNAMIC,
} GINSTR_Datatype;
#define GUIN_is_number_or_int(x)    (x >= GINSTRDATATYPE_INT32 && x <= GINSTRDATATYPE_NUMBER64)
#define GUIN_is_number(x)           (x == GINSTRDATATYPE_NUMBER32 || x == GINSTRDATATYPE_NUMBER64)
#define GUIN_is_int(x)              (x == GINSTRDATATYPE_INT32 || x == GINSTRDATATYPE_INT64)
#define GUIN_is_number_variant(x)   (GUIN_is_number_or_int(x) || x == GINSTRDATATYPE_BOOL || x == GINSTRDATATYPE_CHAR)

#define GUIN_underflow_Bytecode(_bytecode_ptr, ptr)          (ptr < _bytecode_ptr->bytecode)
#define GUIN_overflow_Bytecode(_bytecode_ptr, ptr)           (ptr >= _bytecode_ptr->bytecode + _bytecode_ptr->size)
#define GUIN_out_of_bounds_from_Bytecode(_bytecode_ptr, ptr) (GUIN_overflow_Bytecode(_bytecode_ptr, ptr) || GUIN_underflow_Bytecode(_bytecode_ptr, ptr))

typedef struct GUIN_Bytecode {
    GUIN_ubyte* bytecode;
    size_t length;
    size_t size;
} GUIN_Bytecode;

char* GUIN_GINSTR_Datatype_to_string(GINSTR_Datatype x);

GUIN_Bytecode  GUIN_init_Bytecode(void);
GUIN_Bytecode* GUIN_add_Bytecode(GUIN_Bytecode* x, const GUIN_ubyte* data, const size_t data_length);
GUIN_Bytecode* GUIN_add_Bytecode_one_byte(GUIN_Bytecode* x, const GUIN_ubyte data);
GUIN_Bytecode* GUIN_add_Bytecode_w_byte_size(GUIN_Bytecode* x, const void* data, const size_t length);
GUIN_Bytecode* GUIN_add_Bytecode_String_no_size_embedded(GUIN_Bytecode* x, const GUIN_String* str);
bool GUIN_safe_memcpy_Bytecode(void* destination, GUIN_Bytecode* src, GUIN_ubyte* start, size_t length);
bool GUIN_double_Bytecode_size(GUIN_Bytecode* x);
void GUIN_destroy_Bytecode(GUIN_Bytecode* x);
void GUIN_destroy_Bytecode_ptr(GUIN_Bytecode** x);
bool GUIN_print_Bytecode_into_ASM(GUIN_Bytecode* x);

#include "compiler/ast.h"
GINSTR_Datatype GUIN_ASTDatatype_to_Bytecode_Datatype(GUIN_ASTDatatype x);

#endif
