#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../etc/strings.h"
#include "lexer.h"
#include "ast.h"
#include "../../../include/declarations.h"

typedef enum guin_ScopeType {
    guin_SCOPE_UNINIT = 0,
    guin_SCOPE_GLOBAL,
    guin_SCOPE_FUNCTION,
    guin_SCOPE_IF,


    guin_SCOPE_LOOP_SCOPE_START,
    guin_SCOPE_WHILE,
    guin_SCOPE_FOR,
    guin_SCOPE_LOOP_SCOPE_END,


    guin_SCOPE_SCOPE
} guin_ScopeType;

typedef enum guin_ScopeSearchType {
    guin_SCOPE_SEARCH_GLOBAL,
    guin_SCOPE_SEARCH_FUNCTION,
    guin_SCOPE_SEARCH_LOOP
} guin_ScopeSearchType;

typedef struct guin_ParseScopeNode {
    struct guin_ParseScopeNode* prev;
    guin_ScopeType scopetype;
    guin_ASTDatatype ret_type;

    struct {
        guin_VariableInfoAST* arr;
        size_t size;
        size_t length;
    } var_info;
} guin_ParseScopeNode;


typedef struct guin_ParseState {
    guin_LexToken prev;
    guin_LexToken current;
    guin_LexToken ahead;

    guin_ParseScopeNode*  scope_top;
    guin_ParseScopeNode* scope_base_aka_global;
    guin_LexState lState;
    char* errmsg;
} guin_ParseState;

guin_ParseScopeNode* guin_init_ParseScopeNode_ptr(guin_ScopeType scopetype, guin_ASTDatatype ret_type);
void guin_destroy_ParseScopeNode(guin_ParseScopeNode** pScope);
bool guin_add_ParseScopeNode(guin_ParseState* pState, guin_ScopeType scopetype, guin_ASTDatatype ret_type);
bool guin_pop_ParseScopeNode(guin_ParseState* pState, guin_ASTScope* x);
guin_ParseScopeNode* guin_get_descendant_of_Scope_ParseScopeNode(guin_ParseState* pState, guin_ScopeSearchType searchtype);
bool guin_add_ParseScopeNode_variable(guin_ParseState* pState, guin_String* identifier, guin_ASTDatatype datatype);
guin_VariableInfoAST* guin_get_var_info(guin_ParseState* pState, char* name);
guin_ASTDatatype guin_get_current_tk_datatype_parser(guin_ParseState* pState);

guin_ParseState guin_init_ParseState(guin_String* file_content);
void guin_destroy_ParseState(guin_ParseState* pState);
void guin_advance_parser(guin_ParseState* pState);

//guin_ExpressionNodeType LexTokenEnum_to_ValidExpressionNodeType_Operation(guin_LexTokenEnum x);
//guin_ExpressionAST* guin_get_function_args_in_expression_parser(guin_ParseState* pState);
guin_AST* guin_parse_segment(guin_ParseState* pState, const guin_LexTokenEnum ending, const bool is_global_scope);

#define guin_is_op(LexTokenType) (LexTokenType <= guin_TK_LT_EQU && LexTokenType >= guin_TK_ASSIGN)

#endif
