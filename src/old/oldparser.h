#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../etc/strings.h"
#include "lexer.h"
#include "ast.h"
#include "../../etc/declarations.h"

typedef struct ParseScopeNode ParseScopeNode;

typedef struct {
    LexToken prev;
    LexToken current;
    LexToken ahead;

    ParseScopeNode* scope_top;
    LexState lState;
    char* errmsg;
} ParseState;

typedef enum {
    SCOPE_UNINIT = 0,
    SCOPE_GLOBAL,
    SCOPE_FUNCTION,
    SCOPE_IF,
    SCOPE_WHILE,
    SCOPE_FOR,
    SCOPE_SCOPE
} ScopeType;

struct ParseScopeNode {
    ParseScopeNode* prev;
    ScopeType scopetype;

    struct {
        VariableInfoAST* arr;
        size_t var_info_size;
        size_t var_info_len;
    } var_info;
};

ParseScopeNode init_ParseScopeNode(ScopeType scopetype);
void destroy_ParseScopeNode(ParseScopeNode** pScope);
bool add_ParseScopeNode(ParseState* pState, ScopeType scopetype);
bool pop_ParseScopeNode(ParseState* pState);


ParseState init_ParseState(String* file_content);
void destroy_ParseState(ParseState* pState);
void advance_parser(ParseState* pState);

bool eval_expression_parser(ParseState* pState, LexTokenEnum token_to_signify_end, bool is_global_scope);
bool get_function_args_in_expression_parser(ParseState* pState);
bool eval_if_statement(ParseState* pState);
bool parse_segment(ParseState* pState, LexTokenEnum ending, bool is_global_scope);
bool parse(String* file_content);

#endif