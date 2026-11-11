#ifndef guin_PARSER_h
#define guin_PARSER_h

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../etc/strings.h"
#include "lexer.h"
#include "parse-state.h"
#include "ast.h"
#include "../../../include/declarations.h"

guin_AST* guin_parse_segment(guin_ParseState* pState, const bool in_global_scope);

#define guin_is_op(LexTokenType) (LexTokenType <= guin_TK_LT_EQU && LexTokenType >= guin_TK_ASSIGN)

#endif
