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

GUIN_ParseScopeNode* GUIN_init_ParseScopeNode_ptr(GUIN_ScopeType scopetype)
{
    GUIN_ParseScopeNode* x = malloc(sizeof(GUIN_ParseScopeNode));
    if (!x) return NULL;

    GUIN_ParseScopeNode y = {0};
    y.scopetype = scopetype;
    y.var_info.size = 4;
    y.var_info.arr = malloc(sizeof(GUIN_VariableInfoAST) * y.var_info.size);
    if (!y.var_info.arr) {
        free(x);
        return NULL;
    }

    for (size_t i = 0; i < y.var_info.size; i++)
        y.var_info.arr[i] = (GUIN_VariableInfoAST){0};

    *x = y;
    return x;
}

void GUIN_destroy_ParseScopeNode(GUIN_ParseScopeNode** pScope)
{
    if (!pScope)    return;
    if (!(*pScope)) return;

    GUIN_ParseScopeNode* pScope_ref = *pScope;

    for (size_t i = 0; i < pScope_ref->var_info.length; ++i) {
        GUIN_destroy_VariableInfoAST(&pScope_ref->var_info.arr[i]);
    }

    free(pScope_ref->var_info.arr);
    pScope_ref->var_info.arr = NULL;
    free(pScope_ref);
    *pScope = NULL;
}

bool GUIN_add_ParseScopeNode(GUIN_ParseState* pState, GUIN_ScopeType scopetype)
{
    GUIN_ParseScopeNode* x = GUIN_init_ParseScopeNode_ptr(scopetype);
    if (!x) return false;

    x->prev = pState->scope_top;
    pState->scope_top = x;

    return true;
}

bool GUIN_pop_ParseScopeNode(GUIN_ParseState* pState, GUIN_ASTScope* x)
{
    if (!pState) return false;

    // invalid
    if (!pState->scope_top) return false;
    // this means we are in the global scope
    if (!pState->scope_top->prev) return false;

    // delete
    GUIN_ParseScopeNode* current = pState->scope_top;
    pState->scope_top = current->prev;

    for (size_t i = 0; i < current->var_info.length; ++i) {
        GUIN_log("slot:%d\n", current->var_info.arr[i].slot);
        GUIN_AST toadd = (GUIN_AST){
            .nodetype=GUIN_ASTNODE_CLEAR_LOCAL_SLOT,
            .clearLocalSlotAST.slot = current->var_info.arr[i].slot-1
        };
        GUIN_add_AST_to_ASTScope(x, toadd);
        GUIN_log("added toadd\n");
    }

    GUIN_destroy_ParseScopeNode(&current);
    return true;
}

// deep copies the string
bool GUIN_add_ParseScopeNode_variable(GUIN_ParseState* pState, GUIN_String* identifier, GUIN_ASTDatatype datatype)
{
    if (!pState || !identifier) return false;

    if (!pState->scope_top) {
        GUIN_log("there is no scope top!\n");
        return false;
    }

    GUIN_ParseScopeNode* top = pState->scope_top;

    if (top->var_info.length >= top->var_info.size) {
        size_t original_size = top->var_info.size;
        while (top->var_info.length >= top->var_info.size) top->var_info.size *= 2;
        GUIN_VariableInfoAST* tmp = realloc(top->var_info.arr, top->var_info.size * sizeof(GUIN_VariableInfoAST));
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

    GUIN_VariableInfoAST x = (GUIN_VariableInfoAST){
        .datatype = datatype,
        .identifier = GUIN_copystring(identifier),
        .slot = slot_number
    };
    top->var_info.arr[top->var_info.length++] = x;
    return true;
}

// DO NOT FREE
GUIN_VariableInfoAST* GUIN_get_var_info(GUIN_ParseState* pState, char* name)
{
    if (!pState || !name) return NULL;
    GUIN_ParseScopeNode* current = pState->scope_top;
    while (current) {
        GUIN_log("checking %zu %zu\n", current->var_info.length, current->var_info.size);
        for (size_t i = 0; i < current->var_info.length; ++i) {
            GUIN_log("comparing %s... %d\n", current->var_info.arr[i].identifier.content);
            if (strcmp(current->var_info.arr[i].identifier.content, name) == 0) {
                return &current->var_info.arr[i];
            }
        }
        current = current->prev;
    }
    return NULL;
}



GUIN_ParseState GUIN_init_ParseState(GUIN_String* file_content)
{
    GUIN_ParseState x = {0};
    x.lState = GUIN_init_LexState(file_content);
    x.scope_top = GUIN_init_ParseScopeNode_ptr(GUIN_SCOPE_GLOBAL);
    x.scope_base_aka_global = &x.scope_top;
    return x;
}

void GUIN_destroy_ParseState(GUIN_ParseState* pState)
{
    GUIN_ParseScopeNode* top = pState->scope_top;

    while (top) {
        GUIN_ParseScopeNode* next = top->prev;
        GUIN_destroy_ParseScopeNode(&top);
        top = next;
    }

    GUIN_destroy_LexToken(&pState->prev);
    GUIN_destroy_LexToken(&pState->current);
    GUIN_destroy_LexToken(&pState->ahead);
}

void GUIN_advance_parser(GUIN_ParseState* pState)
{
    if (pState->lState.index != 0) {
        GUIN_destroy_LexToken(&pState->prev);
        pState->prev    = pState->current;
        pState->current = pState->ahead;
        pState->ahead   = GUIN_advance_lexer(&pState->lState);
    } else if (pState->current.type != GUIN_TK_EOF) {
        pState->current = GUIN_advance_lexer(&pState->lState);
        pState->ahead   = GUIN_advance_lexer(&pState->lState);
    }
}

static GUIN_LexTokenEnum valid_Operations[] = {
    GUIN_TK_ADD,
    GUIN_TK_SUB,
    GUIN_TK_MUL,
    GUIN_TK_DIV,
    GUIN_TK_MOD,
    GUIN_TK_POW,

    GUIN_TK_and,
    GUIN_TK_or,

    GUIN_TK_EQU,
    GUIN_TK_NOT_EQU,
    GUIN_TK_GT,
    GUIN_TK_LT,
    GUIN_TK_GT_EQU,
    GUIN_TK_LT_EQU,
};

struct GUIN_ExpressionPrecedenceStruct {
    GUIN_ubyte rank;
    GUIN_ExpressionNodeType type;
};

// 0 == (literal value like int or string)
static struct GUIN_ExpressionPrecedenceStruct exprNodeOps[] = {
    {.rank=1,.type=GUIN_EXPRNODE_AND     },

    {.rank=2,.type=GUIN_EXPRNODE_OR      },
    
    {.rank=3,.type=GUIN_EXPRNODE_EQU     },
    {.rank=3,.type=GUIN_EXPRNODE_NOT_EQU },
    {.rank=3,.type=GUIN_EXPRNODE_GT      },
    {.rank=3,.type=GUIN_EXPRNODE_LT      },
    {.rank=3,.type=GUIN_EXPRNODE_GT_EQU  },
    {.rank=3,.type=GUIN_EXPRNODE_LT_EQU  },

    {.rank=4,.type=GUIN_EXPRNODE_ADD     },
    {.rank=4,.type=GUIN_EXPRNODE_SUB     },

    {.rank=5,.type=GUIN_EXPRNODE_MUL     },
    {.rank=5,.type=GUIN_EXPRNODE_DIV     },
    {.rank=5,.type=GUIN_EXPRNODE_MOD     },

    {.rank=6,.type=GUIN_EXPRNODE_POW     },
};

GUIN_ExpressionNodeType GUIN_LexTokenEnum_to_ValidExpressionNodeType_Operation(GUIN_LexTokenEnum x)
{
    switch (x)
    {
        case GUIN_TK_Number_val:     return GUIN_EXPRNODE_NUMBER;
        case GUIN_TK_Int_val:        return GUIN_EXPRNODE_INT;
        case GUIN_TK_String_val:     return GUIN_EXPRNODE_STRING;
        case GUIN_TK_Char_val:       return GUIN_EXPRNODE_CHAR;
        case GUIN_TK_Bool_val:       return GUIN_EXPRNODE_BOOL;

        case GUIN_TK_ADD:            return GUIN_EXPRNODE_ADD;
        case GUIN_TK_SUB:            return GUIN_EXPRNODE_SUB;
        case GUIN_TK_MUL:            return GUIN_EXPRNODE_MUL;
        case GUIN_TK_DIV:            return GUIN_EXPRNODE_DIV;
        case GUIN_TK_MOD:            return GUIN_EXPRNODE_MOD;
        case GUIN_TK_POW:            return GUIN_EXPRNODE_POW;

        case GUIN_TK_and:            return GUIN_EXPRNODE_AND;
        case GUIN_TK_or:             return GUIN_EXPRNODE_OR;

        case GUIN_TK_GT:             return GUIN_EXPRNODE_GT;
        case GUIN_TK_LT:             return GUIN_EXPRNODE_LT;
        case GUIN_TK_GT_EQU:         return GUIN_EXPRNODE_GT_EQU;
        case GUIN_TK_LT_EQU:         return GUIN_EXPRNODE_LT_EQU;
        case GUIN_TK_EQU:            return GUIN_EXPRNODE_EQU;
        case GUIN_TK_NOT_EQU:        return GUIN_EXPRNODE_NOT_EQU;
        default:                     return GUIN_EXPRNODE_UNINIT;
    }
}

// 0 == (literal value like int or string)
GUIN_ubyte GUIN_get_Precedence_level(GUIN_ExpressionNodeType type)
{
    for (size_t i = 0; i < sizeof(exprNodeOps)/sizeof(struct GUIN_ExpressionPrecedenceStruct); i++)
        if (exprNodeOps[i].type == type) return exprNodeOps[i].rank;
    return 0;
}

static bool GUIN_is_in(GUIN_LexTokenEnum x, GUIN_LexTokenEnum* array, size_t len) 
{
    for (size_t i = 0; i < len; i++) {
        if (x == array[i]) return true;
    }
    return false;
}

#define GUIN_pState_prev    (pState->prev)
#define GUIN_pState_current (pState->current)
#define GUIN_pState_ahead   (pState->ahead)

#define GUIN_is_op(LexTokenType)                                   (GUIN_is_in(LexTokenType, valid_Operations, sizeof(valid_Operations)/sizeof(GUIN_LexTokenEnum)))
#define GUIN_is_EOF_or_end(LexTokenType)                           (LexTokenType == GUIN_TK_EOF || LexTokenType == GUIN_TK_CURLY_R)
#define GUIN_is_function_end(LexTokenType, current_tk_type)        (LexTokenType == GUIN_TK_COMMA && (current_tk_type == GUIN_TK_PARENTHESIS_R || LexTokenType == GUIN_TK_COMMA))
#define GUIN_is_valid_Expr_end(LexTokenType, current_tk_type)      (GUIN_is_EOF_or_end(LexTokenType) || current_tk_type == LexTokenType)

static GUIN_ExpressionAST* GUIN_eval_expression_parser(GUIN_ParseState* pState, const GUIN_LexTokenEnum token_to_signify_end, const bool parsing_func_call_arg, const bool is_global_scope);


// IF IT RETURNS ExpressionAST->error, EXPECT THAT MEANS ExpressionAST->top HAS BEEN DEALT WITH AND IS FREED
static GUIN_ExpressionAST* GUIN_get_value_expression_parser(GUIN_ParseState* pState, const GUIN_LexTokenEnum token_to_signify_end, const bool parsing_func_call_arg, const bool is_global_scope)
{
    if (!pState || !token_to_signify_end) return NULL;

    GUIN_ExpressionAST* exprAST = GUIN_init_ExpressionAST_ptr();
    if (!exprAST) return NULL;
    exprAST->top = NULL;
    
    GUIN_ExpressionNodeAST** current = &exprAST->top; // current exprNode we are on

    // PLAN:
    // WHEN REACHES VALID POINT IT ASSIGNS
    // ELSE EXITS
    // BUT I NEED TO MAINTAIN THAT IT CHANGES THE exprAST correctly when needs
    // but doesnt when it reaches a non continuation point

    bool dont_break  = true;
    bool on_negative = false; 
    while (true) {
        switch (GUIN_pState_current.type)
        {
            case GUIN_TK_nil:
                GUIN_assign_ExpressionNodeAST(current, GUIN_EXPRNODE_NIL);
                dont_break = false;
                break;

            case GUIN_TK_Int_val:
                GUIN_assign_ExpressionNodeAST(current, GUIN_EXPRNODE_INT);
                (*current)->data.integer = GUIN_pState_current.integer;
                dont_break = false;
                break;

            case GUIN_TK_Bool_val:
                GUIN_assign_ExpressionNodeAST(current, GUIN_EXPRNODE_BOOL);
                (*current)->data.bl = GUIN_pState_current.bl;
                dont_break = false;
                break;
            
            case GUIN_TK_SUB: // AUTO ASSUME ITS AT THE START OF A CHAIN OF NEG
                on_negative = !on_negative;
                break;

            case GUIN_TK_Number_val:
                GUIN_assign_ExpressionNodeAST(current, GUIN_EXPRNODE_NUMBER);
                (*current)->data.number = GUIN_pState_current.number;
                dont_break = false;
                break;

            case GUIN_TK_Identifier: {
                GUIN_VariableInfoAST* info = GUIN_get_var_info(pState, GUIN_pState_current.string.content);

                if (!info) {
                    pState->errmsg = "Identifier does not exist!";
                    exprAST->fail = true;
                    break;
                }

                if (is_global_scope && !info->allowed_in_global_expression) {
                    pState->errmsg = "Identifier cannot be used in a global expression!";
                    exprAST->fail = true;
                    break;
                }

                if (info->slot == 0) {
                    GUIN_assign_ExpressionNodeAST(current, GUIN_EXPRNODE_GLOBAL_IDENTIFIER);
                    (*current)->data.string_identifier = GUIN_copystring(&GUIN_pState_current.string);
                } else {
                    GUIN_assign_ExpressionNodeAST(current, GUIN_EXPRNODE_LOCAL_IDENTIFIER);
                    (*current)->data.slot_num = info->slot-1;
                }
                dont_break = false;
                break;
            }

            case GUIN_TK_String_val:
                GUIN_assign_ExpressionNodeAST(current, GUIN_EXPRNODE_STRING);
                (*current)->data.string_identifier = GUIN_copystring(&GUIN_pState_current.string);
                dont_break = false;
                break;

            case GUIN_TK_PARENTHESIS_L:
                GUIN_destroy_ExpressionAST_ptr(&exprAST);
                GUIN_log_push_layer();
                exprAST = GUIN_eval_expression_parser(pState, GUIN_TK_PARENTHESIS_R, false, is_global_scope);
                GUIN_log_pop_layer();
                
                if (!exprAST)      return NULL;    // failed alloc
                if (exprAST->fail) return exprAST; // pass the state over

                GUIN_ExpressionNodeAST* tmp = exprAST->top;
                exprAST->top = NULL;

                GUIN_assign_ExpressionNodeAST(&exprAST->top, GUIN_EXPRNODE_PARENTHESIS);
                exprAST->top->right = tmp;

                dont_break = false;
                GUIN_advance_parser(pState);
                break;

            case GUIN_TK_UNKNOWN:
                GUIN_destroy_ExpressionNodeAST_ptr(&exprAST->top);
                exprAST->fail = true;
                pState->errmsg = pState->lState.errmsg;
                dont_break = false;
                break;

            case GUIN_TK_ERR:
                GUIN_destroy_ExpressionNodeAST_ptr(&exprAST->top);
                exprAST->fail = true;
                pState->errmsg = pState->lState.errmsg;
                dont_break = false;
                break;

            default:
                GUIN_log("got no value (%s)\n", GUIN_LexTokenEnum_to_string(GUIN_pState_current.type));
                if (!(GUIN_is_EOF_or_end(token_to_signify_end) || (parsing_func_call_arg && GUIN_is_function_end(token_to_signify_end, GUIN_pState_current.type)))) {
                    if (GUIN_is_op(GUIN_pState_current.type))
                        pState->errmsg = "This arithmetic cannot be located behind the value!";
                    else
                        pState->errmsg = "Invalid start to an expression!";
                }
                dont_break = false;
                break;
        }
        if (!dont_break || exprAST->fail) break;
        GUIN_advance_parser(pState);
    }

    if (!exprAST->fail) {
        do {
            // func call
            if (GUIN_pState_ahead.type == GUIN_TK_PARENTHESIS_L) {
                GUIN_log("getting a function call!\n");

                GUIN_ExprFuncCallAST func_call = GUIN_init_ExprFuncCallAST();
                if (!func_call.expression_args) {
                    GUIN_destroy_ExpressionAST_ptr(&exprAST);
                    return NULL; // failed alloc
                }

                GUIN_advance_parser(pState);
                GUIN_int16 func_args = 0;
                if (GUIN_pState_ahead.type == GUIN_TK_PARENTHESIS_R && GUIN_pState_current.type == GUIN_TK_PARENTHESIS_L) {
                    GUIN_advance_parser(pState);
                } else {
                    while (GUIN_pState_current.type != GUIN_TK_PARENTHESIS_R) {
                        ++func_args;
                        GUIN_ExpressionAST* eAST = GUIN_eval_expression_parser(pState, GUIN_TK_COMMA, true, is_global_scope);

                        if (func_args > 255) {
                            pState->errmsg = "Overflowed the maximum function arguements allowed (255), stop making poor design choices!";
                            GUIN_destroy_ExpressionNodeAST_ptr(&exprAST->top);
                            GUIN_destroy_ExprFuncCallAST(&func_call);
                            exprAST->fail = true;
                            return exprAST;
                        }
                        if (!eAST) {
                            GUIN_destroy_ExpressionAST_ptr(&exprAST);
                            GUIN_destroy_ExprFuncCallAST(&func_call);
                            return NULL;
                        }
                        if (eAST->fail) {
                            if (!eAST->top && GUIN_pState_current.type == GUIN_TK_COMMA)
                                pState->errmsg = "Expected a valid arguement but it was left empty!";
                            GUIN_destroy_ExpressionNodeAST_ptr(&exprAST->top);
                            GUIN_destroy_ExprFuncCallAST(&func_call);
                            exprAST->fail = true;
                            return exprAST;
                        }
                        if (!eAST->top) {
                            if (GUIN_pState_ahead.type != GUIN_TK_COMMA && GUIN_pState_ahead.type != GUIN_TK_PARENTHESIS_R) {
                                pState->errmsg = "Expected a valid arguement but it was left empty!";
                                GUIN_destroy_ExpressionNodeAST_ptr(&exprAST->top);
                                GUIN_destroy_ExprFuncCallAST(&func_call);
                                exprAST->fail = true;
                                return exprAST;
                            }
                        }
                        else if (!GUIN_add_ExpressionAST_ptr_to_ExprFuncCallAST(&func_call, eAST)) {
                            GUIN_destroy_ExpressionAST_ptr(&exprAST);
                            GUIN_destroy_ExprFuncCallAST(&func_call);
                            return NULL; // failed alloc
                        }

                        GUIN_advance_parser(pState);

                        if (GUIN_pState_current.type == GUIN_TK_COMMA) {
                            if (GUIN_pState_ahead.type == GUIN_TK_COMMA || GUIN_pState_ahead.type == GUIN_TK_PARENTHESIS_R) {
                                GUIN_advance_parser(pState); // for errmsg
                                pState->errmsg = "Expected a valid arguement but it was left empty!";
                                GUIN_destroy_ExpressionNodeAST_ptr(&exprAST->top);
                                GUIN_destroy_ExprFuncCallAST(&func_call);
                                exprAST->fail = true;
                                return exprAST;
                            }
                        }
                    }
                }

                GUIN_ExpressionNodeAST* tmp = NULL;
                GUIN_assign_ExpressionNodeAST(&tmp, GUIN_EXPRNODE_CALL);
                if (!tmp) {
                    GUIN_destroy_ExpressionAST_ptr(&exprAST);
                    return NULL;
                }

                tmp->right = exprAST->top;
                tmp->data.exprFuncCallAST = func_call;
                exprAST->top = tmp;
            }
            else { 
                break;
            }
        } while (true);
        

        if (on_negative) {
            GUIN_ExpressionNodeAST* tmp = NULL;
            GUIN_assign_ExpressionNodeAST(&tmp, GUIN_EXPRNODE_NEG);
            if (!tmp) {
                GUIN_destroy_ExpressionAST_ptr(&exprAST);
                return NULL;
            }
            tmp->right = exprAST->top;
            exprAST->top = tmp;
        }
    }
    else {
        GUIN_destroy_ExpressionNodeAST_ptr(&exprAST->top);
    }

    return exprAST;
}


// IF YOU ONLY RECIEVE NULL, THAT MEANS IT FAILED TO ALLOC MEM
// ELSE TO KNOW IF ITD FAIL, YOU'D CHECK ...->failed
// EXPECT IF IT ->failed THAT ->top IS COMPLETELY INVALID
static GUIN_ExpressionAST* GUIN_eval_expression_parser_section(GUIN_ParseState* pState, const GUIN_LexTokenEnum token_to_signify_end, const bool parsing_func_call_arg, const bool is_global_scope)
{
    GUIN_ExpressionAST* exprAST = GUIN_init_ExpressionAST_ptr();
    if (!exprAST) return NULL;

    GUIN_advance_parser(pState);

    GUIN_log_push_layer();
    GUIN_ExpressionAST* value = GUIN_get_value_expression_parser(pState, token_to_signify_end, parsing_func_call_arg, is_global_scope);
    GUIN_log_pop_layer();
    { // checking the status of the current value, whether it exists or not or whatever
        if (!value) {
            GUIN_log("invalid! (value = NULL)\n");
            exprAST->fail = true;
            return exprAST; // this one is required to return asap

        } else if (value->fail) {
            GUIN_log("invalid! (it failed...)\n");
            //pState->errmsg = "Expression was not finished!";
            exprAST->fail = true;
            GUIN_destroy_ExpressionAST_ptr(&value);
            return exprAST;

        } else if (!value->top) {
            GUIN_log("value doesnt exist!\n");
            GUIN_destroy_ExpressionAST_ptr(&value);
            exprAST->fail = true;
            return exprAST;
        }
    }
    
    { // Checking if its not just negative symbols like this: <val> + -- (nothing after) 
        GUIN_ExpressionNodeAST* check = value->top;

        while (check) {
            if (check->type == GUIN_EXPRNODE_NEG) check = check->right;
            else break;
        }
        if (!check) {
            GUIN_log("check doesnt exist so its invalid!\n");
            pState->errmsg = "Expression was not finished!";
            exprAST->fail = true;
            GUIN_destroy_ExpressionNodeAST_ptr(&exprAST->top);
            GUIN_destroy_ExpressionAST_ptr(&value);
            return exprAST;
        }
    }

    if (GUIN_is_op(GUIN_pState_ahead.type)) {
        GUIN_advance_parser(pState);

        { // merge the value and exprAST variables, making value invalid
            GUIN_assign_ExpressionNodeAST(&exprAST->top, GUIN_LexTokenEnum_to_ValidExpressionNodeType_Operation(GUIN_pState_current.type));
            exprAST->top->left = value->top;

            value->top = NULL;
            GUIN_destroy_ExpressionAST_ptr(&value);
        }
    } else {
        exprAST->top = value->top;
        value->top = NULL;
        GUIN_destroy_ExpressionAST_ptr(&value);
        
        if (parsing_func_call_arg && GUIN_is_function_end(token_to_signify_end, GUIN_pState_ahead.type)) {   
            return exprAST;
        }
        else if (!parsing_func_call_arg && GUIN_is_valid_Expr_end(token_to_signify_end, GUIN_pState_ahead.type)) {
            return exprAST;
        }

        pState->errmsg = "Expected a valid end of the expression!";
        GUIN_destroy_ExpressionNodeAST_ptr(&exprAST->top);
        exprAST->fail = true;
        GUIN_advance_parser(pState);
    }
    return exprAST;
}


static GUIN_ExpressionAST* GUIN_eval_expression_parser(GUIN_ParseState* pState, const GUIN_LexTokenEnum token_to_signify_end, const bool parsing_func_arg_call, const bool is_global_scope)
{
    GUIN_ExpressionAST* main = GUIN_eval_expression_parser_section(pState, token_to_signify_end, parsing_func_arg_call, is_global_scope);
    GUIN_ExpressionNodeAST** current = NULL; // POINTS TO THE CURRENT main BRANCH | IF CURRENT == NULL IT MEANS WE HAVE REACHED THE END OF THE PARSER

    if (!main)
        return NULL;

    if (main->fail) {
        return main;
    }

    if (GUIN_get_Precedence_level(main->top->type) == 0)
        return main;

    current = &main->top;

    do { // IF CURRENT == NULL IT MEANS WE HAVE REACHED THE END OF THE PARSER
        GUIN_ExpressionAST* right;
        
        { // fetching right
            right = GUIN_eval_expression_parser_section(pState, token_to_signify_end, parsing_func_arg_call, is_global_scope);

            if (!right) {
                GUIN_destroy_ExpressionAST_ptr(&right);
                GUIN_destroy_ExpressionNodeAST_ptr(&main->top);
                main->fail = true;
                return main;
            }
            if (!right->top || right->fail) {
                GUIN_destroy_ExpressionAST_ptr(&right);
                GUIN_destroy_ExpressionNodeAST_ptr(&main->top);
                main->fail = true;
                return main;
            }
        }

        GUIN_ExpressionNodeAST* c_ref = *current;
        GUIN_byte current_precedence  = GUIN_get_Precedence_level(c_ref->type);
        GUIN_byte right_precedence    = GUIN_get_Precedence_level(right->top->type);

        if (current_precedence >= right_precedence && right_precedence != 0) {
            // we position main to be on the left side of the lesser/equ right
            c_ref->right     = right->top->left;
            right->top->left = c_ref;
            *current = right->top;
        }
        else if (right_precedence == 0) {
            c_ref->right = right->top;
            current = NULL;
        }
        else { // current_precedence < right_precedence
            c_ref->right = right->top;
            current = &c_ref->right;
        }

        // before it exits
        right->top = NULL;
        GUIN_destroy_ExpressionAST_ptr(&right);
    } while (current);

    return main;
}


GUIN_ASTDatatype GUIN_get_current_tk_datatype_parser(GUIN_ParseState* pState)
{
    GUIN_advance_parser(pState);
    
    switch (pState->current.type)
    {
        case GUIN_TK_char:       return GUIN_ASTDATATYPE_CHAR;
        case GUIN_TK_string:     return GUIN_ASTDATATYPE_STRING;
        case GUIN_TK_int:        return GUIN_ASTDATATYPE_INT;
        case GUIN_TK_number:     return GUIN_ASTDATATYPE_NUMBER;
        case GUIN_TK_bool:       return GUIN_ASTDATATYPE_BOOL;
        case GUIN_TK_func:       return GUIN_ASTDATATYPE_FUNCTION;
        case GUIN_TK_dynamic:    return GUIN_ASTDATATYPE_DYNAMIC;
        case GUIN_TK_void:       return GUIN_ASTDATATYPE_VOID;
        default:                 return GUIN_ASTDATATYPE_ERR;
    }
}



// DECLARATION/ASSIGNMENT
GUIN_AST GUIN_eval_variable_parser(GUIN_ParseState* pState, GUIN_LexTokenEnum ending, bool is_global_scope)
{
    GUIN_advance_parser(pState);

    GUIN_AST ASTNode = {0};
    ASTNode.nodetype = GUIN_ASTNODE_DECLARATION;
    ASTNode.error = false;

    if (GUIN_pState_current.type != GUIN_TK_Identifier) {
        pState->errmsg = "Expected an identifier...";
        ASTNode.error = true;
        return ASTNode;
    }
    ASTNode.declarationAST.info.identifier = GUIN_copystring(&GUIN_pState_current.string);
    GUIN_log("-------------------------------------------------------\n");
    GUIN_log("done copying string %s\n", ASTNode.declarationAST.info.identifier.content);

    if (GUIN_get_var_info(pState, ASTNode.declarationAST.info.identifier.content)) {
        pState->errmsg = "Identifier name already exists!";
        ASTNode.error = true;
        return ASTNode;
    }

    GUIN_log("PARSING DECLARATION FOR [%s]\n", ASTNode.declarationAST.info.identifier.content);

    GUIN_advance_parser(pState);

    if (GUIN_pState_current.type != GUIN_TK_COLON) {
        pState->errmsg = "Expected a colon...";
        ASTNode.error = true;
        return ASTNode;
    }

    ASTNode.declarationAST.info.datatype = GUIN_get_current_tk_datatype_parser(pState);
    if (!ASTNode.declarationAST.info.datatype) {
        pState->errmsg = "Invalid datatype!";
        ASTNode.error = true;
        return ASTNode;
    }
    else if (ASTNode.declarationAST.info.datatype == GUIN_ASTDATATYPE_VOID) {
        pState->errmsg = "Cannot assign a variable as a void datatype!";
        ASTNode.error = true;
        return ASTNode;
    }

    if (GUIN_pState_ahead.type != GUIN_TK_ASSIGN) {
        GUIN_advance_parser(pState); // just for the error
        pState->errmsg = "Expected '='!";
        ASTNode.error = true;
        return ASTNode;
    }
    GUIN_advance_parser(pState);

    GUIN_log("parsing...\n");
    GUIN_log_push_layer();
    ASTNode.declarationAST.expression = GUIN_eval_expression_parser(pState, ending, false, is_global_scope);
    GUIN_log_pop_layer();

    {
        GUIN_log("Expression tree:\n");
        GUIN_log("-------------------\n");
        GUIN_log_push_layer();
        GUIN_log_ExpressionNodeAST(ASTNode.declarationAST.expression->top);
        GUIN_log_pop_layer();
        GUIN_log("-------------------\n");
    }
    GUIN_log("determining if its invalid\n");
    if (!ASTNode.declarationAST.expression->top) {
        ASTNode.error = true;
        GUIN_log("missing expression tree for variable declaration!\n");
        if (!pState->errmsg) pState->errmsg = "Expected an expression! (from declaration)";
    }
    else if (ASTNode.declarationAST.expression->fail) {
        ASTNode.error = true;
    }

    { // this is here as its assumed that this name is going to be used for other places, so not to cause cascading errors
        bool adding_var_status = GUIN_add_ParseScopeNode_variable(pState, &ASTNode.declarationAST.info.identifier, ASTNode.declarationAST.info.datatype);
        if (adding_var_status) {
            GUIN_log("done adding name\n");
            GUIN_log("-------------------------------------------------------\n");
        } else {
            GUIN_log("failed adding name!\n");
            GUIN_log("-------------------------------------------------------\n");
            ASTNode.error = true;
            return ASTNode;
        }
        ASTNode.declarationAST.slot = pState->scope_top->var_info.arr[pState->scope_top->var_info.length-1].slot;
    }
    return ASTNode;
}






static GUIN_ASTScope GUIN_parser_get_scope(GUIN_ParseState* pState, const GUIN_ScopeType scopetype, const GUIN_LexTokenEnum ending)
{
    if (!pState) return (GUIN_ASTScope){0};

    GUIN_ASTScope scope = GUIN_init_ASTScope();
    if (!scope.nodes) return (GUIN_ASTScope){0};

    GUIN_add_ParseScopeNode(pState, scopetype);

    // DO NOT AND I MEAN NOT DESTROY X
    GUIN_AST x;
    while (true) {
        x = GUIN_parse_segment(pState, ending, false);
        
        if (x.error) {
            GUIN_log("has error\n");
            GUIN_destroy_ASTScope(&scope);
            break;
        }
        if (x.nodetype == GUIN_ASTNODE_END)
            break;
        if (x.nodetype == GUIN_ASTNODE_IGNORE)
            continue;
        
        GUIN_add_AST_to_ASTScope(&scope, x);
    }

    if (!x.error) {
        GUIN_log("add all existing variables to nuke\n");
        GUIN_pop_ParseScopeNode(pState, &scope);
        GUIN_log("return scope\n");
    }
    return scope;
}







static GUIN_AST GUIN_eval_if_and_while_statement(GUIN_ParseState* pState)
{
    if (!pState) return (GUIN_AST){0};

    GUIN_log("doing if/while statement\n");

    GUIN_AST x = (GUIN_AST){0};
    x.error = true;

    if (GUIN_pState_current.type == GUIN_TK_if) {
        x.nodetype = GUIN_ASTNODE_IF;
    } 
    else if (GUIN_pState_current.type == GUIN_TK_while) {
        x.nodetype = GUIN_ASTNODE_WHILE;
    }
    else {
        GUIN_log("invalid current token for if/while!\n");
        return x;
    }

    GUIN_log("-------------------------------------------------------\n");
    GUIN_log_push_layer();
    //x.ifWhileAST.expression = eval_expression_parser(pState, (x.nodetype == ASTNODE_IF)? GUIN_TK_then : GUIN_TK_do, false);
    x.ifWhileAST.expression = GUIN_eval_expression_parser(pState, GUIN_TK_CURLY_L, false, false);
    GUIN_log_pop_layer();
    GUIN_log("-------------------------------------------------------\n");
    if (!x.ifWhileAST.expression) return x;
    if (x.ifWhileAST.expression->fail) {
        GUIN_destroy_ExpressionAST_ptr(&x.ifWhileAST.expression);
        return x;
    }

    if (!x.ifWhileAST.expression->top) {
        pState->errmsg = "Expected an expression! (from if)";
        GUIN_destroy_ExpressionAST_ptr(&x.ifWhileAST.expression);
        return x;
    }

    GUIN_log("Statement:\n");
    GUIN_log_ExpressionNodeAST(x.ifWhileAST.expression->top);

    GUIN_advance_parser(pState);

    GUIN_log("-------------------------------------------------------\n");
    GUIN_log_push_layer();
    //x.ifWhileAST.nodes = parser_get_scope(pState, (x.nodetype == ASTNODE_IF)? SCOPE_IF : SCOPE_WHILE, GUIN_TK_end);
    x.ifWhileAST.nodes = GUIN_parser_get_scope(pState, (x.nodetype == GUIN_ASTNODE_IF)? GUIN_SCOPE_IF : GUIN_SCOPE_WHILE, GUIN_TK_CURLY_R);
    GUIN_log_pop_layer();
    GUIN_log("-------------------------------------------------------\n");

    if (!x.ifWhileAST.nodes.nodes) {
        GUIN_destroy_ExpressionAST_ptr(&x.ifWhileAST.expression);
        GUIN_destroy_ASTScope(&x.ifWhileAST.nodes);
        return x;
    }

    x.error = false;
    return x;
}





static GUIN_AST GUIN_eval_scope_statement(GUIN_ParseState* pState)
{
    if (!pState) return (GUIN_AST){0};

    GUIN_log("doing scope statement\n");

    GUIN_AST x = (GUIN_AST){0};
    x.nodetype = GUIN_ASTNODE_SCOPE;
    x.error = true;

    GUIN_log("-------------------------------------------------------\n");
    GUIN_log_push_layer();
    //x.scopeAST = parser_get_scope(pState, SCOPE_SCOPE, GUIN_TK_end);
    x.scopeAST = GUIN_parser_get_scope(pState, GUIN_SCOPE_SCOPE, GUIN_TK_CURLY_R);
    GUIN_log_pop_layer();
    GUIN_log("-------------------------------------------------------\n");

    if (!x.scopeAST.nodes) {
        GUIN_destroy_ASTScope(&x.scopeAST);
        return x;
    }

    x.error = false;
    return x;
}




GUIN_AST GUIN_parse_segment(GUIN_ParseState* pState, const GUIN_LexTokenEnum ending, const bool is_global_scope)
{
    GUIN_advance_parser(pState);
    
    GUIN_log("parsing a new segment\n");

    switch (GUIN_pState_current.type)
    {
        case GUIN_TK_var:
            GUIN_log("doing var\n");
            return GUIN_eval_variable_parser(pState, ending, is_global_scope);

        case GUIN_TK_if:
            if (is_global_scope) {
                pState->errmsg = "Cannot use a local-only statement in the Global scope!";
                return (GUIN_AST){
                    .error=true,
                    .nodetype=GUIN_ASTNODE_IF
                };
            }
            GUIN_log("doing if\n");
            return GUIN_eval_if_and_while_statement(pState);

        case GUIN_TK_while:
            if (is_global_scope) {
                pState->errmsg = "Cannot use a local-only statement in the Global scope!";
                return (GUIN_AST){
                    .error=true,
                    .nodetype=GUIN_ASTNODE_WHILE
                };
            }
            GUIN_log("doing while\n");
            return GUIN_eval_if_and_while_statement(pState);

        //case GUIN_TK_do:
        case GUIN_TK_CURLY_L:
            /*
            if (is_global_scope) {
                pState->errmsg = "Cannot use a local-only statement in the Global scope!";
                return (G_AST){
                    .error=true,
                    .nodetype=ASTNODE_SCOPE
                };
            }
                */
            GUIN_log("doing scope NEW\n");
            return GUIN_eval_scope_statement(pState);

        default: {
            if (GUIN_pState_current.type == GUIN_TK_SEMI_COLON && ending != GUIN_TK_SEMI_COLON) {
                GUIN_log("is semi\n");
                GUIN_AST x = (GUIN_AST){0};
                return x;
            }

            // to tell the IR to stop looping, the scope has closed or the end of the file
            if (GUIN_pState_current.type == ending) {
                GUIN_log("is ending\n");
                GUIN_AST x = (GUIN_AST){0};
                x.nodetype = GUIN_ASTNODE_END;
                return x;
            }

            GUIN_log("no matches!\n");

            if (ending == GUIN_TK_CURLY_R) {
                pState->errmsg = "Expected an end to the scope!";
            } else {
                pState->errmsg = "Expected a valid statement!";
            }

            GUIN_AST x = (GUIN_AST){0};
            x.error = true;
            return x;
        }
    }
}

#endif
