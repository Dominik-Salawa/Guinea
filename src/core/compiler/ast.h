#ifndef AST_H
#define AST_H

#include "../../etc/declarations.h"
#include "../../etc/strings.h"

typedef enum {
    DATATYPE_INT = 0,
    DATATYPE_NUMBER,
    DATATYPE_BOOL,
    DATATYPE_FUNCTION,
    DATATYPE_STRING,
    DATATYPE_CHAR,
    DATATYPE_DYNAMIC
} Datatype;

typedef enum {
    STRUCTYPE_SINGLE = 0,
    STRUCTYPE_DYNAMIC,
    STRUCTYPE_ARRAY,
    //STRUCTYPE_CLASS
} StructureType;

typedef struct {
    String name;
    ubyte structure_type;
    bool is_ptr;

    ubyte* datatypes;
    size_t data_types_size;
} VariableInfoAST;

void destroy_VariableInfoAST(VariableInfoAST* x);

#endif