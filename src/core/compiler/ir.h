#ifndef IR_H
#define IR_H

#include "parser.h"
#include "ast.h"
#include "../bytecode.h"

typedef struct guin_IR {
    char* filename;
    guin_ubyte ver[2];
    guin_String source;
} guin_IR;

GINSTR guin_ExpressionNodeType_to_GINSTR(guin_ExpressionNodeType x);
guin_Bytecode* guin_IR_CONVERT(guin_IR* ir);

#endif
