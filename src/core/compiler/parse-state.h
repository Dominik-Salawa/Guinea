#ifndef guin_PARSE_STATE_h
#define guin_PARSE_STATE_h

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../etc/strings.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "../../../include/declarations.h"

typedef struct guin_ParseState guin_ParseState;

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


typedef struct guin_NamespaceParseNode {
	struct {
    	struct guin_NamespaceParseNode* names;
		size_t length;
		size_t size;
	} subnames;
	char* name;
	struct guin_NamespaceParseNode* prev;
} guin_NamespaceParseNode;

// source is located in src/declarations.c
extern size_t MAX_NAMESPACE_DEPTH;

bool guin_init_NamespaceParseNode(guin_NamespaceParseNode* node, const char* name, guin_NamespaceParseNode* prev);
bool guin_init_NamespaceParseNode_chown_of_name(guin_NamespaceParseNode* node, char* name, guin_NamespaceParseNode* prev);
void guin_destroy_NamespaceParseNode(guin_NamespaceParseNode* x);
// deep copies the string, returns false if it fails to add
guin_NamespaceParseNode* guin_add_NamespaceParseNode_in_Namespace(guin_NamespaceParseNode* parent, const char* name);
// claims ownership of the string, returns false if it fails to add
guin_NamespaceParseNode* guin_add_NamespaceParseNode_in_Namespace_chown_of_name(guin_NamespaceParseNode* parent, char* name);
// DO NOT DESTROY
// recommend to make a tmp array of top to bottom of Namespaces, with the length of it in order to find
// the correct one
// returns NULL if not found
guin_NamespaceParseNode* guin_find_Namespace(guin_ParseState* pState, const char** names, const size_t names_len);
// DO NOT DESTROY
// recommend to make a tmp array of top to bottom of Namespaces, with the length of it in order to find
// the correct one
// returns NULL if not found
guin_NamespaceParseNode* guin_find_Namespace_in_Namespace(guin_NamespaceParseNode* pState, const char** names, const size_t names_len);
void guin_log_Namespace(guin_NamespaceParseNode* parent);

guin_NamespaceParseNode* guin_push_Namespace(guin_ParseState* pState, const char* name);
void guin_pop_Namespace(guin_ParseState* pState);




typedef struct guin_ParseState {
    guin_LexToken prev;
    guin_LexToken current;
    guin_LexToken ahead;

	struct {
		size_t depth;
		guin_NamespaceParseNode* current;
		guin_NamespaceParseNode start; // contains all the nodes and all
	} namespacing;

    guin_ParseScopeNode* scope_top;
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

bool guin_init_ParseState(guin_ParseState* pState, guin_String* file_content);
void guin_destroy_ParseState(guin_ParseState* pState);
void guin_advance_parser(guin_ParseState* pState);

#endif
