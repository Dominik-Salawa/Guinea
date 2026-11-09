#ifndef PARSER_C
#define PARSER_C

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../etc/strings.h"
#include "../../etc/log.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"

guin_ParseScopeNode* guin_init_ParseScopeNode_ptr(guin_ScopeType scopetype, guin_ASTDatatype ret_type)
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

void guin_destroy_ParseScopeNode(guin_ParseScopeNode** pScope)
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

bool guin_add_ParseScopeNode(guin_ParseState* pState, guin_ScopeType scopetype, guin_ASTDatatype ret_type)
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

bool guin_pop_ParseScopeNode(guin_ParseState* pState, guin_ASTScope* x)
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


guin_ParseScopeNode* guin_get_descendant_of_Scope_ParseScopeNode(guin_ParseState* pState, guin_ScopeSearchType searchtype)
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
bool guin_add_ParseScopeNode_variable(guin_ParseState* pState, guin_String* identifier, guin_ASTDatatype datatype)
{
    if (!pState || !identifier) return false;

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
guin_VariableInfoAST* guin_get_var_info(guin_ParseState* pState, char* name)
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



guin_ParseState guin_init_ParseState(guin_String* file_content)
{
    guin_ParseState x = {0};
    x.lState = guin_init_LexState(file_content);
    x.scope_top = guin_init_ParseScopeNode_ptr(guin_SCOPE_GLOBAL, guin_ASTDATATYPE_ERR);
    x.scope_base_aka_global = x.scope_top;
    return x;
}

void guin_destroy_ParseState(guin_ParseState* pState)
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
}

void guin_advance_parser(guin_ParseState* pState)
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

static guin_LexTokenEnum valid_Operations[] = {
    guin_TK_ADD,
    guin_TK_SUB,
    guin_TK_MUL,
    guin_TK_DIV,
    guin_TK_MOD,
    guin_TK_POW,

    guin_TK_and,
    guin_TK_or,

    guin_TK_EQU,
    guin_TK_NOT_EQU,
    guin_TK_GT,
    guin_TK_LT,
    guin_TK_GT_EQU,
    guin_TK_LT_EQU,
	guin_TK_ASSIGN,
};

struct guin_ExpressionPrecedenceStruct {
    guin_ubyte rank;
    guin_ExpressionNodeType type;
};

// 0 == (literal value like int or string)
static struct guin_ExpressionPrecedenceStruct exprNodeOps[] = {
    { .rank=1,.type=guin_EXPRNODE_ASSIGN  },
    
	{ .rank=2,.type=guin_EXPRNODE_AND     },

    { .rank=3,.type=guin_EXPRNODE_OR      },
    
    { .rank=4,.type=guin_EXPRNODE_EQU     },
    { .rank=4,.type=guin_EXPRNODE_NOT_EQU },
    { .rank=4,.type=guin_EXPRNODE_GT      },
    { .rank=4,.type=guin_EXPRNODE_LT      },
    { .rank=4,.type=guin_EXPRNODE_GT_EQU  },
    { .rank=4,.type=guin_EXPRNODE_LT_EQU  },

    { .rank=5,.type=guin_EXPRNODE_ADD     },
    { .rank=5,.type=guin_EXPRNODE_SUB     },

    { .rank=6,.type=guin_EXPRNODE_MUL     },
    { .rank=6,.type=guin_EXPRNODE_DIV     },
    { .rank=6,.type=guin_EXPRNODE_MOD     },

    { .rank=7,.type=guin_EXPRNODE_POW     },

};

guin_ExpressionNodeType guin_LexTokenEnum_to_ValidExpressionNodeType_Operation(guin_LexTokenEnum x)
{
    switch (x)
    {
        case guin_TK_Number_val:     return guin_EXPRNODE_NUMBER;
        case guin_TK_Int_val:        return guin_EXPRNODE_INT;
        case guin_TK_String_val:     return guin_EXPRNODE_STRING;
        case guin_TK_Char_val:       return guin_EXPRNODE_CHAR;
        case guin_TK_Bool_val:       return guin_EXPRNODE_BOOL;

        case guin_TK_ADD:            return guin_EXPRNODE_ADD;
        case guin_TK_SUB:            return guin_EXPRNODE_SUB;
        case guin_TK_MUL:            return guin_EXPRNODE_MUL;
        case guin_TK_DIV:            return guin_EXPRNODE_DIV;
        case guin_TK_MOD:            return guin_EXPRNODE_MOD;
        case guin_TK_POW:            return guin_EXPRNODE_POW;

        case guin_TK_and:            return guin_EXPRNODE_AND;
        case guin_TK_or:             return guin_EXPRNODE_OR;

        case guin_TK_GT:             return guin_EXPRNODE_GT;
        case guin_TK_LT:             return guin_EXPRNODE_LT;
        case guin_TK_GT_EQU:         return guin_EXPRNODE_GT_EQU;
        case guin_TK_LT_EQU:         return guin_EXPRNODE_LT_EQU;
        case guin_TK_EQU:            return guin_EXPRNODE_EQU;
        case guin_TK_NOT_EQU:        return guin_EXPRNODE_NOT_EQU;
		case guin_TK_ASSIGN:		 return guin_EXPRNODE_ASSIGN;
		default: return guin_EXPRNODE_UNINIT;
    }
}

// 0 == (literal value like int or string)
guin_ubyte guin_get_Precedence_level(guin_ExpressionNodeType type)
{
    for (size_t i = 0; i < sizeof(exprNodeOps)/sizeof(struct guin_ExpressionPrecedenceStruct); i++)
        if (exprNodeOps[i].type == type) return exprNodeOps[i].rank;
    return 0;
}

#define guin_pState_prev    (pState->prev)
#define guin_pState_current (pState->current)
#define guin_pState_ahead   (pState->ahead)

#define guin_is_EOF_or_end(LexTokenType)                           (LexTokenType == guin_TK_EOF || LexTokenType == guin_TK_CURLY_R)
#define guin_is_function_end(LexTokenType, current_tk_type)        (LexTokenType == guin_TK_COMMA && (current_tk_type == guin_TK_PARENTHESIS_R || LexTokenType == guin_TK_COMMA))
#define guin_is_valid_Expr_end(LexTokenType, current_tk_type)      (guin_is_EOF_or_end(LexTokenType) || current_tk_type == LexTokenType)

/*

guin_ASTScope guin_parser_get_scope(guin_ParseState* pState, const bool push_new_scopetype, const guin_ScopeType scopetype, const guin_LexTokenEnum ending, const bool enforce_left_curly);

#define guin_eval_function_expression_parser_CLEANUP_PARSE_SCOPE_NODES()\
    false_top->prev = NULL;\
    false_top = pState->scope_top;\
    while (false_top) {\
        guin_ParseScopeNode* tmp = false_top->prev;\
        guin_destroy_ParseScopeNode(&false_top);\
        false_top = tmp;\
    }\
    pState->scope_top = old


	
// NULL == failed mem
static guin_ExpressionAST* guin_eval_function_expression_parser(guin_ParseState* pState, const bool name_required, const bool from_statement)
{
    if (!pState) return NULL;
    guin_advance_parser(pState);

    guin_ExpressionAST* exprAST = guin_init_ExpressionAST_ptr();
    if (!exprAST) return NULL;
    exprAST->top = NULL;

    guin_FunctionAST func = guin_init_FunctionAST();
    if (!func.args.args) {
        guin_destroy_ExpressionAST_ptr(&exprAST);
        return NULL;
    }

    if (guin_pState_current.type == guin_TK_Identifier) {
        func.name = guin_copystring_as_ptr(&guin_pState_current.string);
        if (!func.name) {
            guin_destroy_ExpressionAST_ptr(&exprAST);
            guin_destroy_FunctionAST(&func);
            return NULL;
        }
        
        guin_log("Checking if name exists...\n");

        // 0 == global, also function names should ONLY be able to 
        guin_VariableInfoAST* var = guin_get_var_info(pState, func.name->content);

        if (!from_statement) {
            if (var && var->slot == 0) {
                guin_destroy_ExpressionAST(exprAST);
                guin_destroy_FunctionAST(&func);
                exprAST->fail = true;
                pState->errmsg = "Identifier already exists in the Global scope!";
                guin_log("function identifier already exists in the Global scope!\n");
                return exprAST;
            }
        }
        else if (var) {
            guin_destroy_ExpressionAST(exprAST);
            guin_destroy_FunctionAST(&func);
            exprAST->fail = true;
            pState->errmsg = "Identifier already exists!";
            guin_log("function identifier already exists!\n");
            return exprAST;
        }
    }
    else if (name_required) {
        guin_destroy_ExpressionAST_ptr(&exprAST);
        guin_destroy_FunctionAST(&func);
        exprAST->fail = true;
        pState->errmsg = "Expected an Identifier!";
        return exprAST;
    }

    if (guin_pState_current.type == guin_TK_Identifier)
        guin_advance_parser(pState);
    
    // for now no args
    if (guin_pState_current.type != guin_TK_PARENTHESIS_L) {
        guin_destroy_ExpressionAST(exprAST);
        guin_destroy_FunctionAST(&func);
        exprAST->fail = true;
        pState->errmsg = "Expected `(`!";
        return exprAST;
    }
    guin_advance_parser(pState);
    if (guin_pState_current.type != guin_TK_PARENTHESIS_R) {
        guin_destroy_ExpressionAST(exprAST);
        guin_destroy_FunctionAST(&func);
        exprAST->fail = true;
        pState->errmsg = "Expected `)`!";
        return exprAST;
    }

    guin_advance_parser(pState);
    if (guin_pState_current.type != guin_TK_COLON) {
        guin_destroy_ExpressionAST(exprAST);
        guin_destroy_FunctionAST(&func);
        exprAST->fail = true;
        pState->errmsg = "Expected `:`!";
        return exprAST;
    }
    guin_log("getting datatype...\n");

    func.return_type = guin_get_current_tk_datatype_parser(pState);
    if (func.return_type == guin_ASTDATATYPE_ERR) {
        guin_destroy_ExpressionAST(exprAST);
        guin_destroy_FunctionAST(&func);
        exprAST->fail = true;
        pState->errmsg = "Expected a valid datatype!";
        guin_log("Expected a valid datatype!\n");
        return exprAST;
    }
    guin_log("got datatype\n");

    guin_advance_parser(pState);
    guin_log("Replacing current ParseScopeNodes with a new fresh one...\n");

    guin_ParseScopeNode* old = pState->scope_top;
    pState->scope_top = pState->scope_base_aka_global;

    if (!guin_add_ParseScopeNode(pState, guin_SCOPE_FUNCTION, func.return_type)) {
        pState->scope_top = old;
        guin_destroy_ExpressionAST_ptr(&exprAST);
        guin_destroy_FunctionAST(&func);
        return NULL;
    }
    guin_ParseScopeNode* false_top = pState->scope_top; // this is so later on we can detach the ScopeNode we just made

    if (func.name && !guin_add_ParseScopeNode_variable(pState, func.name, guin_ASTDATATYPE_FUNCTION)) {
        guin_destroy_ExpressionAST_ptr(&exprAST);
        guin_destroy_FunctionAST(&func);
        guin_eval_function_expression_parser_CLEANUP_PARSE_SCOPE_NODES();
        return NULL;
    }

    guin_log("parsing function\n");
    guin_log_push_layer();
    guin_ASTScope scope = guin_parser_get_scope(pState, false, 0, guin_TK_CURLY_R, true);
    guin_log_pop_layer();

    guin_eval_function_expression_parser_CLEANUP_PARSE_SCOPE_NODES();
    
    if (scope.error) {
        guin_destroy_ASTScope(&scope);
        guin_destroy_ExpressionAST(exprAST);
        guin_destroy_FunctionAST(&func);
        exprAST->fail = true;
        return exprAST;
    }

    if (!scope.nodes) {
        guin_destroy_ExpressionAST_ptr(&exprAST);
        guin_destroy_FunctionAST(&func);
        return NULL;
    }

    guin_log("func eval done %zu | %s %s %s\n", scope.length, guin_LexTokenEnum_to_string(guin_pState_prev.type), guin_LexTokenEnum_to_string(guin_pState_current.type), guin_LexTokenEnum_to_string(guin_pState_ahead.type));
    func.scope = scope;

    guin_assign_ExpressionNodeAST(&exprAST->top, guin_EXPRNODE_FUNCTION_LITERAL);
    exprAST->top->data.function = func;
    return exprAST;
}


guin_ASTDatatype guin_get_current_tk_datatype_parser(guin_ParseState* pState)
{    
    switch (pState->current.type)
    {
        case guin_TK_char:       return guin_ASTDATATYPE_CHAR;
        case guin_TK_string:     return guin_ASTDATATYPE_STRING;
        case guin_TK_int:        return guin_ASTDATATYPE_INT;
        case guin_TK_number:     return guin_ASTDATATYPE_NUMBER;
        case guin_TK_bool:       return guin_ASTDATATYPE_BOOL;
        case guin_TK_func:       return guin_ASTDATATYPE_FUNCTION;
        case guin_TK_dynamic:    return guin_ASTDATATYPE_DYNAMIC;
        case guin_TK_void:       return guin_ASTDATATYPE_VOID;
        default:                 return guin_ASTDATATYPE_ERR;
    }
}



// DECLARATION/ASSIGNMENT
guin_AST* guin_eval_variable_parser(guin_ParseState* pState, guin_LexTokenEnum ending, bool is_global_scope)
{
    guin_advance_parser(pState);

    guin_AST* ASTNode = guin_init_AST_ptr(guin_ASTNODE_DECLARATION);
    if (!ASTNode) return NULL;

    if (guin_pState_current.type != guin_TK_Identifier) {
        pState->errmsg = "Expected an identifier!";
        ASTNode->error = true;
        return ASTNode;
    }
    ASTNode->declarationAST.info.identifier = guin_copystring(&guin_pState_current.string);
    if (!ASTNode->declarationAST.info.identifier.content) {
        guin_destroy_AST_ptr(&ASTNode);
        return ASTNode;
    }

    guin_log("-------------------------------------------------------\n");
    guin_log("done copying string %s\n", ASTNode->declarationAST.info.identifier.content);

    if (guin_get_var_info(pState, ASTNode->declarationAST.info.identifier.content)) {
        pState->errmsg = "Identifier name already exists!";
        ASTNode->error = true;
        return ASTNode;
    }

    guin_log("PARSING DECLARATION FOR [%s]\n", ASTNode->declarationAST.info.identifier.content);

    guin_advance_parser(pState);

    if (guin_pState_current.type != guin_TK_COLON) {
        pState->errmsg = "Expected `:`!";
        guin_log("Expected `:`!\n");
        ASTNode->error = true;
        return ASTNode;
    }

    guin_advance_parser(pState);

    ASTNode->declarationAST.info.datatype = guin_get_current_tk_datatype_parser(pState);
    if (ASTNode->declarationAST.info.datatype == guin_ASTDATATYPE_ERR) {
        pState->errmsg = "Invalid datatype!";
        guin_log("Invalid datatype!\n");
        ASTNode->error = true;
        return ASTNode;
    }
    else if (ASTNode->declarationAST.info.datatype == guin_ASTDATATYPE_VOID) {
        pState->errmsg = "Cannot assign a variable as a void datatype!";
        guin_log("Cannot assign a variable as a void datatype!\n");
        ASTNode->error = true;
        return ASTNode;
    }

    guin_advance_parser(pState);

    if (guin_pState_current.type != guin_TK_ASSIGN) {
        pState->errmsg = "Expected `=`!";
        guin_log("Expected `=`!\n");
        ASTNode->error = true;
        return ASTNode;
    }

    guin_log("parsing...\n");
    guin_log_push_layer();
    ASTNode->declarationAST.expression = guin_eval_expression_parser(pState, ending, false, is_global_scope, false);
    guin_log_pop_layer();

    {
        guin_log("Expression tree:\n");
        guin_log("-------------------\n");
        guin_log_push_layer();
        guin_log_ExpressionNodeAST(ASTNode->declarationAST.expression->top);
        guin_log_pop_layer();
        guin_log("-------------------\n");
    }
	
    if (!ASTNode->declarationAST.expression->top) {
        ASTNode->error = true;
        guin_log("missing expression tree for variable declaration!\n");
        if (!pState->errmsg) pState->errmsg = "Expected an expression! (from declaration)";
    }
    else if (ASTNode->declarationAST.expression->fail) {
        ASTNode->error = true;
    }

    { // this is here as its assumed that this name is going to be used for other places, so not to cause cascading errors
        bool adding_var_status = guin_add_ParseScopeNode_variable(pState, &ASTNode->declarationAST.info.identifier, ASTNode->declarationAST.info.datatype);
        if (adding_var_status) {
            guin_log("done adding name\n");
            guin_log("-------------------------------------------------------\n");
        } else {
            guin_log("failed adding name!\n");
            guin_log("-------------------------------------------------------\n");
            guin_destroy_AST_ptr(&ASTNode);
            return ASTNode;
        }
        ASTNode->declarationAST.slot = pState->scope_top->var_info.arr[pState->scope_top->var_info.length-1].slot;
    }
    return ASTNode;
}
static guin_AST* guin_eval_function_statement(guin_ParseState* pState)
{
    guin_AST* x = guin_init_AST_ptr(guin_ASTNODE_DECLARATION);
    if (!pState) return x;
    guin_ExpressionAST* funcval = guin_eval_function_expression_parser(pState, true, true);
    if (!funcval) {
        guin_destroy_AST_ptr(&x);
        return x;
    }
    if (funcval->fail) {
        x->error = true;
        guin_destroy_ExpressionAST_ptr(&funcval);
        return x;
    }
    x->declarationAST.expression      = funcval;
    x->declarationAST.info.datatype   = guin_ASTDATATYPE_FUNCTION;
    x->declarationAST.info.identifier = guin_copystring(funcval->top->data.function.name);
    { // this is here as its assumed that this name is going to be used for other places, so not to cause cascading errors
        bool adding_var_status = guin_add_ParseScopeNode_variable(pState, funcval->top->data.function.name, x->declarationAST.info.datatype);
        if (adding_var_status) {
            guin_log("done adding name\n");
            guin_log("-------------------------------------------------------\n");
        } else {
            guin_log("failed adding name!\n");
            guin_log("-------------------------------------------------------\n");
            guin_destroy_AST_ptr(&x);
            return x;
        }
        x->declarationAST.slot = pState->scope_top->var_info.arr[pState->scope_top->var_info.length-1].slot;
    }
    return x;
}





// check if scope.nodes exists first, as that means a mem fault
static guin_ASTScope guin_parser_get_scope(guin_ParseState* pState, const bool push_new_scopetype, const guin_ScopeType scopetype, const guin_LexTokenEnum ending, const bool enforce_left_curly)
{
    if (!pState) return (guin_ASTScope){0};

    guin_ASTScope scope = guin_init_ASTScope();
    if (!scope.nodes) return (guin_ASTScope){0};

    if (enforce_left_curly && guin_pState_current.type != guin_TK_CURLY_L) {
        pState->errmsg = "Expected `{`!";
        scope.error = true;
        return scope;
    }

    if (push_new_scopetype)
        guin_add_ParseScopeNode(pState, scopetype, guin_ASTDATATYPE_ERR);

    // DO NOT AND I MEAN NOT DESTROY X
    guin_AST* x;
    while (true) {
        x = guin_parse_segment(pState, ending, false);
        if (!x) {
            guin_log("main parser get scope mem fault\n");
            guin_destroy_ASTScope(&scope);
            return scope;
        }
        
        if (x->nodetype == guin_ASTNODE_END)
            break;
        if (x->nodetype == guin_ASTNODE_IGNORE) {
            guin_destroy_AST_ptr(&x);
            continue;
        }
        if (x->error) {
            guin_log("scope parsing has error: %d\n", x->nodetype);
            guin_destroy_ASTScope(&scope);
            guin_destroy_AST_ptr(&x);
            scope.error = true;
            break;
        }
        
        guin_add_AST_to_ASTScope(&scope, x);
        guin_mfree(x);
    }
    if (x) guin_mfree(x);

    if (push_new_scopetype) {
        guin_log("add all existing variables to nuke\n");
        guin_pop_ParseScopeNode(pState, &scope);
        guin_log("return scope\n");
    }
    return scope;
}







static guin_AST* guin_eval_if_and_while_statement(guin_ParseState* pState)
{
    if (!pState) return NULL;

    guin_log("doing if/while statement\n");

    guin_AST* x = guin_init_AST_ptr(guin_ASTNODE_IF);
    if (!x) return NULL;
    x->error = true;

    if (guin_pState_current.type == guin_TK_if) {
        x->nodetype = guin_ASTNODE_IF;
    } 
    else if (guin_pState_current.type == guin_TK_while) {
        x->nodetype = guin_ASTNODE_WHILE;
    }
    else {
        pState->errmsg = "invalid current token for if/while!";
        guin_log("invalid current token for if/while!\n");
        return x;
    }

    guin_log("-------------------------------------------------------\n");
    guin_log_push_layer();
    x->ifWhileAST.expression = guin_eval_expression_parser(pState, guin_TK_CURLY_L, false, false, false);
    guin_log_pop_layer();
    guin_log("-------------------------------------------------------\n");
    if (!x->ifWhileAST.expression) {
        guin_destroy_AST_ptr(&x);
        guin_log("mem err if/while expression\n");
        return NULL;
    }
    if (x->ifWhileAST.expression->fail) {
        guin_destroy_ExpressionAST_ptr(&x->ifWhileAST.expression);
        guin_log("error if/while expression\n");
        return x;
    }
    if (!x->ifWhileAST.expression->top) {
        if (!pState->errmsg) pState->errmsg = "Expected an expression! (from if)";
        guin_destroy_ExpressionAST_ptr(&x->ifWhileAST.expression);
        return x;
    }

    guin_log("Statement:\n");
    guin_log_ExpressionNodeAST(x->ifWhileAST.expression->top);

    //guin_advance_parser(pState);

    guin_log("-------------------------------------------------------\n");
    guin_log_push_layer();
    x->ifWhileAST.nodes = guin_parser_get_scope(pState, true, (x->nodetype == guin_ASTNODE_IF)? guin_SCOPE_IF : guin_SCOPE_WHILE, guin_TK_CURLY_R, false);
    guin_log_pop_layer();
    guin_log("-------------------------------------------------------\n");

    if (x->ifWhileAST.nodes.error) {
        x->error = true;
        guin_destroy_IfWhileAST(&x->ifWhileAST);
        return x;
    }
    else if (!x->ifWhileAST.nodes.nodes) {
        guin_destroy_ExpressionAST_ptr(&x->ifWhileAST.expression);
        guin_destroy_ASTScope(&x->ifWhileAST.nodes);
        guin_destroy_AST_ptr(&x);
        return x;
    }

    x->error = false;
    return x;
}


static guin_AST* guin_eval_scope_statement(guin_ParseState* pState)
{
    if (!pState) return NULL;

    guin_log("doing scope statement\n");

    guin_AST* x = guin_init_AST_ptr(guin_ASTNODE_SCOPE);
    if (!x) return NULL;

    guin_log("-------------------------------------------------------\n");
    guin_log_push_layer();
    x->scopeAST = guin_parser_get_scope(pState, true, guin_SCOPE_SCOPE, guin_TK_CURLY_R, true);
    guin_log_pop_layer();
    guin_log("-------------------------------------------------------\n");

    if (x->scopeAST.error) {
        guin_log("Node got an error!\n");
        guin_destroy_ASTScope(&x->scopeAST);
        x->error = true;
        return x;
    }
    if (!x->scopeAST.nodes) {
        guin_log("Node got a mem leak!\n");
        guin_destroy_ASTScope(&x->scopeAST);
        guin_destroy_AST_ptr(&x);
        return NULL;
    }

    return x;
}



static inline guin_AST* guin_eval_break(guin_ParseState* pState)
{
    return guin_init_AST_ptr(guin_ASTNODE_BREAK);
}
static inline guin_AST* guin_eval_continue(guin_ParseState* pState)
{
    return guin_init_AST_ptr(guin_ASTNODE_CONTINUE);
}
// must be previously guaranteed that its in a function or it will segfault
static guin_AST* guin_eval_return(guin_ParseState* pState, const guin_LexTokenEnum ending)
{
    if (!pState) return NULL;
    
    guin_AST* x = guin_init_AST_ptr(guin_ASTNODE_RETURN);
    if (!x) return NULL;

    guin_ExpressionAST* exprAST = guin_eval_expression_parser(pState, ending, false, false, false);

    if (!exprAST) {
        guin_destroy_AST_ptr(&x);
        return NULL;
    }
    else if (exprAST->fail) {
        guin_destroy_ExpressionAST_ptr(&exprAST);
        guin_destroy_AST(x);
        x->error = true;
        guin_log("error func\n");
        return x;
    }
    
    guin_ParseScopeNode* scope_node = guin_get_descendant_of_Scope_ParseScopeNode(pState, guin_SCOPE_SEARCH_FUNCTION);

    if (scope_node->ret_type != guin_ASTDATATYPE_VOID && !exprAST->top) {
        pState->errmsg = "Return cannot have an empty statement if its a non-void function!";
        guin_destroy_AST(x);
        x->error = true;
        return x;
    }
    else if (scope_node->ret_type == guin_ASTDATATYPE_VOID && exprAST->top) {
        pState->errmsg = "Return cannot have a statement if its a void function!";
        guin_destroy_AST(x);
        x->error = true;
        return x;
    }
    x->returnAST.expression = exprAST->top;
    exprAST->top = NULL;
    guin_destroy_ExpressionAST_ptr(&exprAST);
    return x;
}

#define guin_only_local_parser(dt)\
if (is_global_scope) {\
    pState->errmsg = "Cannot use a local only statement in the Global scope!";\
    guin_AST* x = guin_init_AST_ptr(dt);\
    if (!x) return NULL;\
    x->error = true;\
    return x;\
}



guin_AST* guin_parse_segment(guin_ParseState* pState, const guin_LexTokenEnum ending, const bool is_global_scope)
{
	guin_advance_parser(pState);

	switch (guin_pState_current.type)
	{
		case guin_TK_var:
			return guin_eval_variable_parser(pState, ending, is_global_scope);
		default:
			return NULL;
	}
	return NULL;
}

*/

#endif
