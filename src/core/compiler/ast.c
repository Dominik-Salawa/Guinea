#ifndef AST_C
#define AST_C

#include "ast.h"
#include "../../etc/strings.h"

void destroy_VariableInfoAST(VariableInfoAST* x)
{
    free(x->datatypes);
    clearstring(&x->name);
}

#endif