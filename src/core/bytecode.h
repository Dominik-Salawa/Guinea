#ifndef BYTECODE_H
#define BYTECODE_H

#include "../etc/declarations.h"
#include <stdlib.h>

typedef enum GINSTR {
    GINSTR_NULL = 0,
    GINSTR_DECLARE_GLOBAL = 0x01,
    GINSTR_PUSH_GLOBAL,
    GINSTR_PUSH_LOCAL,
    GINSTR_PUSH_IMMEDIATE,

    // operations
    GINSTR_ADD = 0x10,
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

    GINSTR_JNT = 0x30, // Jump Not True
    GINSTR_JNTS,       // Jump Not True SHORT (<=255 bytes away)
} GINSTR;

typedef enum GINSTR_Datatype {
    GINSTRDATATYPE_NIL = 0,
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

typedef struct {
    G_ubyte* bytecode;
    size_t length;
    size_t size;
} G_Bytecode;

char* G_Bytecode_Datatype_to_str(GINSTR_Datatype x)
{
    switch (x)
    {
        case GINSTRDATATYPE_NIL:        return "nil";
        case GINSTRDATATYPE_STRING:     return "string";
        case GINSTRDATATYPE_INT32:      return "int32";
        case GINSTRDATATYPE_INT64:      return "int64";
        case GINSTRDATATYPE_NUMBER32:   return "num32";
        case GINSTRDATATYPE_NUMBER64:   return "num64";
        case GINSTRDATATYPE_BOOL:       return "bool";
        case GINSTRDATATYPE_CHAR:       return "char";
        case GINSTRDATATYPE_FUNCTION:   return "function";
        case GINSTRDATATYPE_DYNAMIC:    return "dynamic";
        default: return "(null)";
    }
}

G_Bytecode init_G_Bytecode();
G_Bytecode* add_G_Bytecode(G_Bytecode* x, const G_ubyte* data, const size_t data_length);
G_Bytecode* add_G_Bytecode_one_byte(G_Bytecode* x, const G_ubyte data);
G_Bytecode* add_G_Bytecode_w_byte_size(G_Bytecode* x, const void* data, const size_t length);
G_Bytecode* add_G_Bytecode_String_no_size_embedded(G_Bytecode* x, const String* str);
bool double_G_Bytecode_size(G_Bytecode* x);
void destroy_G_Bytecode(G_Bytecode* x);
void destroy_G_Bytecode_ptr(G_Bytecode** x);


#include "compiler/ast.h"
GINSTR_Datatype ASTDatatype_to_G_Bytecode_Datatype(ASTDatatype x)
{
    switch (x)
    {
        case ASTDATATYPE_NIL:           return GINSTRDATATYPE_NIL;
        case ASTDATATYPE_STRING:        return GINSTRDATATYPE_STRING;
        case ASTDATATYPE_INT:           return GINSTRDATATYPE_INT64;
        case ASTDATATYPE_NUMBER:        return GINSTRDATATYPE_NUMBER64;
        case ASTDATATYPE_BOOL:          return GINSTRDATATYPE_BOOL;
        case ASTDATATYPE_CHAR:          return GINSTRDATATYPE_CHAR;
        case ASTDATATYPE_FUNCTION:      return GINSTRDATATYPE_FUNCTION;
        case ASTDATATYPE_DYNAMIC:       return GINSTRDATATYPE_DYNAMIC;
        default:
            printf("Couldnt find G_BytecodeDatatype equivalent of ASTDatatype %d!\n", x);
            exit(1);
    }
}

#endif