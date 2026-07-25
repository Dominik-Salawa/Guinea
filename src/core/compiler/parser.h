#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../etc/strings.h"
#include "lexer.h"
#include "ast.h"
#include "../../../include/declarations.h"

typedef enum {
    GUIN_SCOPE_UNINIT = 0,
    GUIN_SCOPE_GLOBAL,
    GUIN_SCOPE_FUNCTION,
    GUIN_SCOPE_IF,
    GUIN_SCOPE_WHILE,
    GUIN_SCOPE_FOR,
    GUIN_SCOPE_SCOPE
} GUIN_ScopeType;

typedef struct GUIN_ParseScopeNode {
    struct GUIN_ParseScopeNode* prev;
    GUIN_ScopeType scopetype;

    struct {
        GUIN_VariableInfoAST* arr;
        size_t size;
        size_t length;
    } var_info;
} GUIN_ParseScopeNode;


typedef struct GUIN_ParseState {
    GUIN_LexToken prev;
    GUIN_LexToken current;
    GUIN_LexToken ahead;

    GUIN_ParseScopeNode*  scope_top;
    GUIN_ParseScopeNode** scope_base_aka_global;
    GUIN_LexState lState;
    char* errmsg;
} GUIN_ParseState;

GUIN_ParseScopeNode* GUIN_init_ParseScopeNode_ptr(GUIN_ScopeType scopetype);
void GUIN_destroy_ParseScopeNode(GUIN_ParseScopeNode** pScope);
bool GUIN_add_ParseScopeNode(GUIN_ParseState* pState, GUIN_ScopeType scopetype);
bool GUIN_pop_ParseScopeNode(GUIN_ParseState* pState, GUIN_ASTScope* x);


GUIN_ParseState GUIN_init_ParseState(GUIN_String* file_content);
void GUIN_destroy_ParseState(GUIN_ParseState* pState);
void GUIN_advance_parser(GUIN_ParseState* pState);

GUIN_ExpressionNodeType LexTokenEnum_to_ValidExpressionNodeType_Operation(GUIN_LexTokenEnum x);
//GUIN_ExpressionAST* GUIN_get_function_args_in_expression_parser(GUIN_ParseState* pState);
GUIN_AST GUIN_parse_segment(GUIN_ParseState* pState, const GUIN_LexTokenEnum ending, const bool is_global_scope);

#endif
