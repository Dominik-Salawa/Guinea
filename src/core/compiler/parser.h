#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../etc/strings.h"
#include "lexer.h"
#include "ast.h"
#include "../../etc/declarations.h"

typedef enum {
    SCOPE_UNINIT = 0,
    SCOPE_GLOBAL,
    SCOPE_FUNCTION,
    SCOPE_IF,
    SCOPE_WHILE,
    SCOPE_FOR,
    SCOPE_SCOPE
} ScopeType;

typedef struct ParseScopeNode {
    struct ParseScopeNode* prev;
    ScopeType scopetype;

    struct {
        VariableInfoAST* arr;
        size_t var_info_size;
        size_t var_info_len;
    } var_info;
} ParseScopeNode;


typedef struct {
    LexToken prev;
    LexToken current;
    LexToken ahead;

    ParseScopeNode*  scope_top;
    ParseScopeNode** scope_base_aka_global;
    LexState lState;
    char* errmsg;
} ParseState;

ParseScopeNode init_ParseScopeNode(ScopeType scopetype);
void destroy_ParseScopeNode(ParseScopeNode** pScope);
bool add_ParseScopeNode(ParseState* pState, ScopeType scopetype);
bool pop_ParseScopeNode(ParseState* pState);


ParseState init_ParseState(String* file_content);
void destroy_ParseState(ParseState* pState);
void advance_parser(ParseState* pState);

ExpressionNodeType LexTokenEnum_to_ValidExpressionNodeType_Operation(LexTokenEnum x);

ExpressionAST* get_function_args_in_expression_parser(ParseState* pState);
G_AST parse_segment(ParseState* pState, const LexTokenEnum ending, const bool is_global_scope);

#endif