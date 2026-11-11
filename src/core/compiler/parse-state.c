#ifndef guin_PARSE_STATE_c
#define guin_PARSE_STATE_c

#include <stdbool.h>
#include "../../../include/declarations.h"
#include "parse-state.h"
#include "../../etc/log.h"

guin_ParseScopeNode* 
guin_init_ParseScopeNode_ptr(guin_ScopeType scopetype, guin_ASTDatatype ret_type)
{
    guin_ParseScopeNode* x = guin_malloc(sizeof(guin_ParseScopeNode));
    if (!x) return NULL;

    guin_ParseScopeNode y = (guin_ParseScopeNode){0};
    y.scopetype = scopetype;
    y.ret_type = ret_type;
    y.var_info.size = 4;
    y.var_info.arr = guin_malloc(sizeof(guin_VariableInfoAST) * y.var_info.size);
    if (!y.var_info.arr) {
        guin_mfree(x);
        return NULL;
    }

    for (size_t i = 0; i < y.var_info.size; i++)
        y.var_info.arr[i] = (guin_VariableInfoAST){0};

    *x = y;
    return x;
}

void
guin_destroy_ParseScopeNode(guin_ParseScopeNode** pScope)
{
    if (!pScope)    return;
    if (!(*pScope)) return;

    guin_ParseScopeNode* pScope_ref = *pScope;

    for (size_t i = 0; i < pScope_ref->var_info.length; ++i) {
        guin_destroy_VariableInfoAST(&pScope_ref->var_info.arr[i]);
    }

    guin_mfree(pScope_ref->var_info.arr);
    pScope_ref->var_info.arr = NULL;
    guin_mfree(pScope_ref);
    *pScope = NULL;
}

bool
guin_add_ParseScopeNode(guin_ParseState* pState, guin_ScopeType scopetype, guin_ASTDatatype ret_type)
{
    guin_ParseScopeNode* x = guin_init_ParseScopeNode_ptr(scopetype, ret_type);
    if (!x) return false;

    guin_log("adding scope...\n");
    x->prev = pState->scope_top;
    pState->scope_top = x;

    if (guin_log_on) {
        int count = 0;
        guin_ParseScopeNode* tmp = x;
        guin_log("Layers:\n");
        while (tmp) {
            ++count;
            guin_log(" - %d\n", tmp->scopetype);
            tmp = tmp->prev;
        }
        guin_log("Scope depth: %d\n", count);
    }

    return true;
}

bool 
guin_pop_ParseScopeNode(guin_ParseState* pState, guin_ASTScope* x)
{
    if (!pState) return false;

    // invalid
    if (!pState->scope_top) return false;
    // this means we are in the global scope
    if (!pState->scope_top->prev) return false;

    // delete
    guin_ParseScopeNode* current = pState->scope_top;
    pState->scope_top = current->prev;

    for (size_t i = 0; i < current->var_info.length; ++i) {
        guin_log("slot:%d\n", current->var_info.arr[i].slot);
        guin_AST toadd = (guin_AST){
            .nodetype=guin_ASTNODE_CLEAR_LOCAL_SLOT,
            .clearLocalSlotAST.slot = current->var_info.arr[i].slot-1
        };
        guin_add_AST_to_ASTScope(x, &toadd);
        guin_log("added toadd\n");
    }

    guin_destroy_ParseScopeNode(&current);
    return true;
}


guin_ParseScopeNode* 
guin_get_descendant_of_Scope_ParseScopeNode(guin_ParseState* pState, guin_ScopeSearchType searchtype)
{
    if (!pState) return NULL;

    if (searchtype == guin_SCOPE_SEARCH_GLOBAL)
        return pState->scope_base_aka_global;
        
    for (guin_ParseScopeNode* pScope = pState->scope_top; pScope != NULL; pScope = pScope->prev) {
        if (searchtype == guin_SCOPE_SEARCH_LOOP && (pScope->scopetype > guin_SCOPE_LOOP_SCOPE_START && pScope->scopetype < guin_SCOPE_LOOP_SCOPE_END)) {
            return pScope;
        }
        else if (searchtype == guin_SCOPE_SEARCH_FUNCTION && pScope->scopetype == guin_SCOPE_FUNCTION) {
            return pScope;
        }
    }

    return NULL;
}

// deep copies the string
bool 
guin_add_ParseScopeNode_variable(guin_ParseState* pState, guin_String* identifier, guin_ASTDatatype datatype)
{
    if (!pState || !identifier) return false;

	if (identifier->length == 0) {
		guin_log("identifier has no length!\n");
		return false;
	}

    if (!pState->scope_top) {
        guin_log("there is no scope top!\n");
        return false;
    }

    guin_ParseScopeNode* top = pState->scope_top;

    if (top->var_info.length >= top->var_info.size) {
        size_t original_size = top->var_info.size;
        while (top->var_info.length >= top->var_info.size) top->var_info.size *= 2;
        guin_VariableInfoAST* tmp = guin_remalloc(top->var_info.arr, top->var_info.size * sizeof(guin_VariableInfoAST));
        if (!tmp) {
            top->var_info.size = original_size;
            return false;
        }
        top->var_info.arr = tmp;
    }

    int slot_number;
    if (top->prev) {
        // a local scope value only here
        while (true) {
            if (top->var_info.length > 0 && top->prev) {
                slot_number = top->var_info.arr[top->var_info.length-1].slot+1;
                break;
            } 
            else if (!top->prev) { 
                // this means we have gone so far down that it required us to reach the global scope, 
                // meaning its the only local variable currently existing
                slot_number = 1;
                break;
            }
            else {
                top = top->prev;
            }
        }
        top = pState->scope_top;
    } else {
        slot_number = 0;
    }

    guin_VariableInfoAST x = (guin_VariableInfoAST){
        .datatype = datatype,
        .identifier = guin_copystring(identifier),
        .slot = slot_number
    };
    top->var_info.arr[top->var_info.length++] = x;
    return true;
}

// DO NOT guin_mfree
guin_VariableInfoAST* 
guin_get_var_info(guin_ParseState* pState, char* name)
{
    if (!pState || !name) return NULL;
    guin_ParseScopeNode* current = pState->scope_top;
    while (current) {
        guin_log("checking %zu %zu\n", current->var_info.length, current->var_info.size);
        for (size_t i = 0; i < current->var_info.length; ++i) {
            guin_log("comparing %s... %d\n", current->var_info.arr[i].identifier.content);
            if (strcmp(current->var_info.arr[i].identifier.content, name) == 0) {
                return &current->var_info.arr[i];
            }
        }
        current = current->prev;
    }
    return NULL;
}



bool 
guin_init_ParseState(guin_ParseState* pState, guin_String* file_content)
{
	if (!pState || !file_content) return false;

    guin_ParseState x = {0};
	if (!guin_init_LexState(&x.lState, file_content))
		return false;

    x.scope_top = guin_init_ParseScopeNode_ptr(guin_SCOPE_GLOBAL, guin_ASTDATATYPE_NULL);
	if (!x.scope_top) return false;

    x.scope_base_aka_global = x.scope_top;
	if (!guin_init_NamespaceParseNode(&x.namespacing.start, NULL, NULL)) {
		guin_destroy_ParseState(&x);
		return false;
	}
	*pState = x;
    return true;
}

void 
guin_destroy_ParseState(guin_ParseState* pState)
{
    guin_ParseScopeNode* top = pState->scope_top;

    while (top) {
        guin_ParseScopeNode* next = top->prev;
        guin_destroy_ParseScopeNode(&top);
        top = next;
    }

    guin_destroy_LexToken(&pState->prev);
    guin_destroy_LexToken(&pState->current);
    guin_destroy_LexToken(&pState->ahead);
	guin_destroy_LexState(&pState->lState);
	guin_destroy_NamespaceParseNode(&pState->namespacing.start);
}

void 
guin_advance_parser(guin_ParseState* pState)
{
    if (pState->lState.index != 0) {
        guin_destroy_LexToken(&pState->prev);
        pState->prev    = pState->current;
        pState->current = pState->ahead;
        pState->ahead   = guin_advance_lexer(&pState->lState);
    } else if (pState->current.type != guin_TK_EOF) {
        pState->current = guin_advance_lexer(&pState->lState);
        pState->ahead   = guin_advance_lexer(&pState->lState);
    }
}


bool 
guin_init_NamespaceParseNode(guin_NamespaceParseNode* node, const char* name, guin_NamespaceParseNode* prev)
{
	if (!node) return false;
	if (name) {
		size_t n = strlen(name);
		char* mem = guin_malloc(n);
		if (!mem) return false;
		for (size_t i = 0; i < n; ++i) 
			mem[i] = name[i];
		return guin_init_NamespaceParseNode_chown_of_name(node, mem, prev);
	}
	else {
		return guin_init_NamespaceParseNode_chown_of_name(node, NULL, prev);
	}
}
void 
guin_destroy_NamespaceParseNode(guin_NamespaceParseNode* x)
{
	if (x->subnames.names) {
		for (size_t i = 0; i < x->subnames.length; ++i)
			guin_destroy_NamespaceParseNode(&x->subnames.names[i]);
	}

	if (x->name) guin_mfree(x->name);
	if (x->subnames.names) guin_mfree(x->subnames.names);
}

// will take ownership of name
bool
guin_init_NamespaceParseNode_chown_of_name(guin_NamespaceParseNode* node, char* name, guin_NamespaceParseNode* prev)
{
	if (!node) return false;
	*node = (guin_NamespaceParseNode){0};
	node->name = name;
	node->prev = prev;

	node->subnames.size = 1;
	node->subnames.names = guin_malloc(node->subnames.size * sizeof(guin_NamespaceParseNode));
	if (!node->subnames.names) {
		if (name) guin_mfree(name);
		return NULL;
	}

	return node;
}

// deep copies the string, returns false if it fails to add
guin_NamespaceParseNode*
guin_add_NamespaceParseNode_in_Namespace(guin_NamespaceParseNode* parent, const char* name)
{
	if (!parent || !name) return false;
	size_t n = strlen(name);
	char* mem = guin_malloc(n+1);
	if (!mem) return false;
	for (size_t i = 0; i < n; ++i) 
		mem[i] = name[i];
	mem[n] = 0;
	return guin_add_NamespaceParseNode_in_Namespace_chown_of_name(parent, mem);
}
// claims ownership of the string, returns false if it fails to add
// assumed it uses the same allocator as Guinea
guin_NamespaceParseNode*
guin_add_NamespaceParseNode_in_Namespace_chown_of_name(guin_NamespaceParseNode* parent, char* name)
{
	if (!parent || !name) return false;
	size_t n = strlen(name);

	if (!parent->subnames.names) {
		if (name) guin_mfree(name);
		return NULL;
	}

	if (parent->subnames.length >= parent->subnames.size) {
		parent->subnames.size *= 2;
		guin_NamespaceParseNode* tmp = guin_remalloc(parent->subnames.names, parent->subnames.size*sizeof(guin_NamespaceParseNode));
		if (!tmp) {
			parent->subnames.size /= 2;
			if (name) guin_mfree(name);
			return NULL;
		}
		parent->subnames.names = tmp;
	}

	return 
		(guin_init_NamespaceParseNode_chown_of_name(&parent->subnames.names[parent->subnames.length], name, parent))? 
			&parent->subnames.names[parent->subnames.length++]
		:
			NULL;
}
// DO NOT DESTROY
// recommend to make a tmp array of top to bottom of Namespaces, with the length of it in order to find
// the correct one
// returns NULL if not found
guin_NamespaceParseNode* 
guin_find_Namespace(guin_ParseState* pState, const char** names, const size_t names_len)
{
	if (!pState || !names || names_len == 0) return NULL;

	for (size_t i = 0; i < pState->namespacing.start.subnames.length; ++i) {
		guin_NamespaceParseNode* x = guin_find_Namespace_in_Namespace(&pState->namespacing.start.subnames.names[i], names, names_len);
		if (x) return x;
	}

	return NULL;
}
// DO NOT DESTROY
// recommend to make a tmp array of top to bottom of Namespaces, with the length of it in order to find
// the correct one
// returns NULL if not found
guin_NamespaceParseNode* 
guin_find_Namespace_in_Namespace(guin_NamespaceParseNode* parent, const char** names, const size_t names_len)
{
	if (!parent || !names || names_len == 0) return NULL;

	for (size_t i = 0; i < parent->subnames.length; ++i) {
		if (strcmp(names[0], parent->subnames.names[i].name) == 0) {
			if (names_len == 1)
				return &parent->subnames.names[i];
			else
				return guin_find_Namespace_in_Namespace(&parent->subnames.names[i], names+1, names_len-1);
		}
	}

	return NULL;
}

void 
guin_log_Namespace(guin_NamespaceParseNode* parent)
{
	if (!parent) {
		guin_log("(null)\n");
		return;
	}
	guin_log_push_layer();
	for (size_t i = 0; i < parent->subnames.length; ++i)
		guin_log_Namespace(&parent->subnames.names[i]);
	guin_log_pop_layer();
}


guin_NamespaceParseNode*
guin_push_Namespace(guin_ParseState* pState, const char* name)
{
	if (!pState || !name)
		return false;

	if (pState->namespacing.depth > MAX_NAMESPACE_DEPTH)
		return false;

	size_t n = strlen(name);
	char* str = guin_malloc(n);
	if (!str) return false;

	for (size_t i = 0; i < n; ++i)
		str[i] = name[i];

	return guin_add_NamespaceParseNode_in_Namespace_chown_of_name(pState->namespacing.current, str);
}
guin_NamespaceParseNode* 
guin_push_Namespace_chown_of_name(guin_ParseState* pState, char* name)
{
	if (!pState || !name) {
		if (name) guin_mfree(name);
		return NULL;
	}

	if (pState->namespacing.depth > MAX_NAMESPACE_DEPTH) {
		if (name) guin_mfree(name);
		return NULL;
	}

	guin_NamespaceParseNode* x = guin_add_NamespaceParseNode_in_Namespace_chown_of_name(pState->namespacing.current, name);
	if (x) {
		++pState->namespacing.depth;
		pState->namespacing.current = &pState->namespacing.current->subnames.names[pState->namespacing.current->subnames.length];
		return x;
	}

	return NULL;
}
void 
guin_pop_Namespace(guin_ParseState* pState)
{
	
}


#endif
