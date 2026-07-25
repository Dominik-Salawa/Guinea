#ifndef IR_H
#define IR_H

#include "parser.h"
#include "ast.h"
#include "../bytecode.h"

typedef struct GUIN_IR {
    char* filename;
    GUIN_ubyte ver[2];
    GUIN_String source;
} GUIN_IR;

GINSTR GUIN_ExpressionNodeType_to_GINSTR(GUIN_ExpressionNodeType x);
GUIN_Bytecode* GUIN_IR_CONVERT(GUIN_IR* ir, const bool on_global);

#endif
