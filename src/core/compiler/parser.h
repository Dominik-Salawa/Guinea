#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../etc/strings.h"
#include "lexer.h"
#include "ast.h"
#include "../../../include/declarations.h"

typedef enum GUIN_ScopeType {
    GUIN_SCOPE_UNINIT = 0,
    GUIN_SCOPE_GLOBAL,
    GUIN_SCOPE_FUNCTION,
    GUIN_SCOPE_IF,


    GUIN_SCOPE_LOOP_SCOPE_START,
    GUIN_SCOPE_WHILE,
    GUIN_SCOPE_FOR,
    GUIN_SCOPE_LOOP_SCOPE_END,


    GUIN_SCOPE_SCOPE
} GUIN_ScopeType;

typedef enum GUIN_ScopeSearchType {
    GUIN_SCOPE_SEARCH_GLOBAL,
    GUIN_SCOPE_SEARCH_FUNCTION,
    GUIN_SCOPE_SEARCH_LOOP
} GUIN_ScopeSearchType;

typedef struct GUIN_ParseScopeNode {
    struct GUIN_ParseScopeNode* prev;
    GUIN_ScopeType scopetype;
    GUIN_ASTDatatype ret_type;

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
    GUIN_ParseScopeNode* scope_base_aka_global;
    GUIN_LexState lState;
    char* errmsg;
} GUIN_ParseState;

GUIN_ParseScopeNode* GUIN_init_ParseScopeNode_ptr(GUIN_ScopeType scopetype, GUIN_ASTDatatype ret_type);
void GUIN_destroy_ParseScopeNode(GUIN_ParseScopeNode** pScope);
bool GUIN_add_ParseScopeNode(GUIN_ParseState* pState, GUIN_ScopeType scopetype, GUIN_ASTDatatype ret_type);
bool GUIN_pop_ParseScopeNode(GUIN_ParseState* pState, GUIN_ASTScope* x);
GUIN_ParseScopeNode* GUIN_get_descendant_of_Scope_ParseScopeNode(GUIN_ParseState* pState, GUIN_ScopeSearchType searchtype);
bool GUIN_add_ParseScopeNode_variable(GUIN_ParseState* pState, GUIN_String* identifier, GUIN_ASTDatatype datatype);
GUIN_VariableInfoAST* GUIN_get_var_info(GUIN_ParseState* pState, char* name);
GUIN_ASTDatatype GUIN_get_current_tk_datatype_parser(GUIN_ParseState* pState);

GUIN_ParseState GUIN_init_ParseState(GUIN_String* file_content);
void GUIN_destroy_ParseState(GUIN_ParseState* pState);
void GUIN_advance_parser(GUIN_ParseState* pState);

//GUIN_ExpressionNodeType LexTokenEnum_to_ValidExpressionNodeType_Operation(GUIN_LexTokenEnum x);
//GUIN_ExpressionAST* GUIN_get_function_args_in_expression_parser(GUIN_ParseState* pState);
GUIN_AST* GUIN_parse_segment(GUIN_ParseState* pState, const GUIN_LexTokenEnum ending, const bool is_global_scope);

#endif
