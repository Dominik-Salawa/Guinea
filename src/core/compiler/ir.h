#ifndef IR_H
#define IR_H

#include "parser.h"
#include "../bytecode.h"

typedef struct G_IR {
    char* filename;
    G_ubyte ver[2];
    String source;
} G_IR;

GINSTR ExpressionNodeType_to_GINSTR(ExpressionNodeType x);
G_Bytecode* G_IR_CONVERT(G_IR* ir, G_ubyte SIZE_T_OF_PLATFORM, const bool on_global);

#endif