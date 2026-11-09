#ifndef guin_BYTECODE_h
#define guin_BYTECODE_h

#include "../../include/declarations.h"
#include "../etc/strings.h"
#include "compiler/ast.h"
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

	GINSTR_SET_IREG,
	GINSTR_PUSH_IREG,
	GINSTR_POP,

    // operations
    GINSTR_ADD = 0x30,
    GINSTR_SUB,
    GINSTR_MUL,
    GINSTR_DIV,
    GINSTR_MOD,
    GINSTR_POW,

    GINSTR_NOT,
    GINSTR_NEG,
    GINSTR_CALL,

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
#define guin_is_number_or_int(x)    (x >= GINSTRDATATYPE_INT32 && x <= GINSTRDATATYPE_NUMBER64)
#define guin_is_number(x)           (x == GINSTRDATATYPE_NUMBER32 || x == GINSTRDATATYPE_NUMBER64)
#define guin_is_int(x)              (x == GINSTRDATATYPE_INT32 || x == GINSTRDATATYPE_INT64)
#define guin_is_number_variant(x)   (guin_is_number_or_int(x) || x == GINSTRDATATYPE_BOOL || x == GINSTRDATATYPE_CHAR)

#define guin_underflow_Bytecode(_bytecode_ptr, ptr)          (ptr < _bytecode_ptr->bytecode)
#define guin_overflow_Bytecode(_bytecode_ptr, ptr)           (ptr >= _bytecode_ptr->bytecode + _bytecode_ptr->length)
#define guin_out_of_bounds_from_Bytecode(_bytecode_ptr, ptr) (guin_overflow_Bytecode(_bytecode_ptr, ptr) || guin_underflow_Bytecode(_bytecode_ptr, ptr))

typedef struct guin_Bytecode {
    guin_ubyte* bytecode;
    size_t length;
    size_t size;
} guin_Bytecode;

char* guin_GINSTR_Datatype_to_string(GINSTR_Datatype x);

guin_Bytecode  guin_init_Bytecode(void);
guin_Bytecode* guin_add_Bytecode(guin_Bytecode* x, const guin_ubyte* data, const size_t data_length);
guin_Bytecode* guin_add_Bytecode_one_byte(guin_Bytecode* x, const guin_ubyte data);
guin_Bytecode* guin_add_Bytecode_w_byte_size(guin_Bytecode* x, const void* data, const size_t length);
guin_Bytecode* guin_add_Bytecode_String_no_size_embedded(guin_Bytecode* x, const guin_String* str);
bool guin_safe_memcpy_Bytecode(void* destination, guin_Bytecode* src, guin_ubyte* start, size_t length);
bool guin_double_Bytecode_size(guin_Bytecode* x);
void guin_destroy_Bytecode(guin_Bytecode* x);
void guin_destroy_Bytecode_ptr(guin_Bytecode** x);
bool guin_print_Bytecode_into_ASM(guin_Bytecode* x);

GINSTR_Datatype guin_ASTDatatype_to_Bytecode_Datatype(guin_ASTDatatype x);

#endif
