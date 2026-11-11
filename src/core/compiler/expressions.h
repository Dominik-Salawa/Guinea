#ifndef guin_EXPRESSIONS_h
#define guin_EXPRESSIONS_h

#include <stdbool.h>
#include "ast.h"
#include "parser.h" 

// Must start at the token where the expression should start, if it fails there, it wont move the position
// If it is returned as incomplete, it means it was able to chain for a bit but was broken, if that is
// the case, expect the Parse position to be a token ahead of the broken chain
guin_ExpressionAST* guin_eval_expression_parser(guin_ParseState* pState, const bool in_global_scope);

#endif
