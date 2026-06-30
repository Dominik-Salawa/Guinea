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

ParseScopeNode* init_ParseScopeNode_ptr(ScopeType scopetype)
{
    ParseScopeNode* x = malloc(sizeof(ParseScopeNode));
    if (!x) return NULL;

    ParseScopeNode y = {0};
    y.scopetype = scopetype;
    y.var_info.size = 4;
    y.var_info.arr = malloc(sizeof(VariableInfoAST) * y.var_info.size);
    if (!y.var_info.arr) {
        free(x);
        return NULL;
    }

    for (size_t i = 0; i < y.var_info.size; i++)
        y.var_info.arr[i] = (VariableInfoAST){0};

    *x = y;
    return x;
}

void destroy_ParseScopeNode(ParseScopeNode** pScope)
{
    if (!pScope)    return;
    if (!(*pScope)) return;

    ParseScopeNode* pScope_ref = *pScope;

    for (size_t i = 0; i < pScope_ref->var_info.length; ++i) {
        destroy_VariableInfoAST(&pScope_ref->var_info.arr[i]);
    }

    free(pScope_ref->var_info.arr);
    pScope_ref->var_info.arr = NULL;
    free(pScope_ref);
    *pScope = NULL;
}

bool add_ParseScopeNode(ParseState* pState, ScopeType scopetype)
{
    ParseScopeNode* x = init_ParseScopeNode_ptr(scopetype);
    if (!x) return false;

    x->prev = pState->scope_top;
    pState->scope_top = x;

    return true;
}

bool pop_ParseScopeNode(ParseState* pState, ASTScope* x)
{
    if (!pState) return false;

    // invalid
    if (!pState->scope_top) return false;
    // this means we are in the global scope
    if (!pState->scope_top->prev) return false;

    // delete
    ParseScopeNode* current = pState->scope_top;
    pState->scope_top = current->prev;

    for (size_t i = 0; i < current->var_info.length; ++i) {
        G_log("slot:%d\n", current->var_info.arr[i].slot);
        G_AST toadd = (G_AST){
            .nodetype=ASTNODE_CLEAR_LOCAL_SLOT,
            .clearLocalSlotAST.slot = current->var_info.arr[i].slot-1
        };
        add_G_AST_to_ASTScope(x, toadd);
    }

    for (size_t i = 0; i < x->length; ++i) {
        switch (x->nodes[i].nodetype) {
            case ASTNODE_IGNORE:                 G_log("ignore\n"); break;
            case ASTNODE_END:                    G_log("end\n"); break;
            case ASTNODE_ASSIGN:                 G_log("assign\n"); break;
            case ASTNODE_CLEAR_LOCAL_SLOT:       G_log("CLEAR_LOCAL_SLOT\n"); break;
            case ASTNODE_IF:                     G_log("if\n"); break;
            case ASTNODE_WHILE:                  G_log("while\n"); break;
            case ASTNODE_SCOPE:                  G_log("scope\n"); break;
            case ASTNODE_DECLARATION:            G_log("declaration\n"); break;
        }
    }

    destroy_ParseScopeNode(&current);
    return true;
}

// deep copies the string
bool add_ParseScopeNode_variable(ParseState* pState, String* identifier, ASTDatatype datatype)
{
    if (!pState || !identifier) return false;

    if (!pState->scope_top) {
        G_log("there is no scope top!\n");
        return false;
    }

    ParseScopeNode* top = pState->scope_top;

    if (top->var_info.length >= top->var_info.size) {
        size_t original_size = top->var_info.size;
        while (top->var_info.length >= top->var_info.size) top->var_info.size *= 2;
        VariableInfoAST* tmp = realloc(top->var_info.arr, top->var_info.size * sizeof(VariableInfoAST));
        if (!tmp) {
            top->var_info.size = original_size;
            return false;
        }
        top->var_info.arr = tmp;
    }

    // we can assume theres at least one Global name, due to main() function being needed in order to run a file
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

    VariableInfoAST x = (VariableInfoAST){
        .datatype = datatype,
        .identifier = copystring(identifier),
        .slot = slot_number
    };
    top->var_info.arr[top->var_info.length++] = x;
    return true;
}

// DO NOT FREE
VariableInfoAST* get_var_info(ParseState* pState, char* name)
{
    if (!pState || !name) return NULL;
    ParseScopeNode* current = pState->scope_top;
    while (current) {
        G_log("checking %zu %zu\n", current->var_info.length, current->var_info.size);
        for (size_t i = 0; i < current->var_info.length; ++i) {
            G_log("comparing %s... %d\n", current->var_info.arr[i].identifier.content);
            if (strcmp(current->var_info.arr[i].identifier.content, name) == 0) {
                return &current->var_info.arr[i];
            }
        }
        current = current->prev;
    }
    return NULL;
}



ParseState init_ParseState(String* file_content)
{
    ParseState x = {0};
    x.lState = init_LexState(file_content);
    x.scope_top = init_ParseScopeNode_ptr(SCOPE_GLOBAL);
    x.scope_base_aka_global = &x.scope_top;
    return x;
}

void destroy_ParseState(ParseState* pState)
{
    ParseScopeNode* top = pState->scope_top;

    while (top) {
        ParseScopeNode* next = top->prev;
        destroy_ParseScopeNode(&top);
        top = next;
    }

    destroy_LexToken(&pState->prev);
    destroy_LexToken(&pState->current);
    destroy_LexToken(&pState->ahead);
}

void advance_parser(ParseState* pState)
{
    if (pState->lState.index != 0) {
        destroy_LexToken(&pState->prev);
        pState->prev    = pState->current;
        pState->current = pState->ahead;
        pState->ahead   = advance_lexer(&pState->lState);
    } else if (pState->current.type != TK_EOF) {
        pState->current = advance_lexer(&pState->lState);
        pState->ahead   = advance_lexer(&pState->lState);
    }
}

static LexTokenEnum valid_Operations[] = {
    TK_ADD,
    TK_SUB,
    TK_MUL,
    TK_DIV,
    TK_MOD,
    TK_POW,

    TK_and,
    TK_or,

    TK_EQU,
    TK_NOT_EQU,
    TK_GT,
    TK_LT,
    TK_GT_EQU,
    TK_LT_EQU,
};

struct ExpressionPrecedenceStruct {
    G_ubyte rank;
    ExpressionNodeType type;
};

// 0 == (literal value like int or string)
static struct ExpressionPrecedenceStruct exprNodeOps[] = {
    {.rank=1,.type=EXPRNODE_AND     },

    {.rank=2,.type=EXPRNODE_OR      },
    
    {.rank=3,.type=EXPRNODE_EQU     },
    {.rank=3,.type=EXPRNODE_NOT_EQU },
    {.rank=3,.type=EXPRNODE_GT      },
    {.rank=3,.type=EXPRNODE_LT      },
    {.rank=3,.type=EXPRNODE_GT_EQU  },
    {.rank=3,.type=EXPRNODE_LT_EQU  },

    {.rank=4,.type=EXPRNODE_ADD     },
    {.rank=4,.type=EXPRNODE_SUB     },

    {.rank=5,.type=EXPRNODE_MUL     },
    {.rank=5,.type=EXPRNODE_DIV     },
    {.rank=5,.type=EXPRNODE_MOD     },

    {.rank=6,.type=EXPRNODE_POW     },
};

ExpressionNodeType LexTokenEnum_to_ValidExpressionNodeType_Operation(LexTokenEnum x)
{
    switch (x)
    {
        case TK_Number_val:     return EXPRNODE_NUMBER;
        case TK_Int_val:        return EXPRNODE_INT;
        case TK_String_val:     return EXPRNODE_STRING;
        case TK_Char_val:       return EXPRNODE_CHAR;
        case TK_Bool_val:       return EXPRNODE_BOOL;

        case TK_ADD:            return EXPRNODE_ADD;
        case TK_SUB:            return EXPRNODE_SUB;
        case TK_MUL:            return EXPRNODE_MUL;
        case TK_DIV:            return EXPRNODE_DIV;
        case TK_MOD:            return EXPRNODE_MOD;
        case TK_POW:            return EXPRNODE_POW;

        case TK_and:            return EXPRNODE_AND;
        case TK_or:             return EXPRNODE_OR;

        case TK_GT:             return EXPRNODE_GT;
        case TK_LT:             return EXPRNODE_LT;
        case TK_GT_EQU:         return EXPRNODE_GT_EQU;
        case TK_LT_EQU:         return EXPRNODE_LT_EQU;
        case TK_EQU:            return EXPRNODE_EQU;
        case TK_NOT_EQU:        return EXPRNODE_NOT_EQU;
        default:                return EXPRNODE_UNINIT;
    }
}

// 0 == (literal value like int or string)
static G_ubyte get_Precedence_level(ExpressionNodeType type)
{
    for (size_t i = 0; i < sizeof(exprNodeOps)/sizeof(struct ExpressionPrecedenceStruct); i++)
        if (exprNodeOps[i].type == type) return exprNodeOps[i].rank;
    return 0;
}

static bool is_in(LexTokenEnum x, LexTokenEnum* array, size_t len) 
{
    for (int i = 0; i < len; i++) {
        if (x == array[i]) return true;
    }
    return false;
}

#define pState_prev    (pState->prev)
#define pState_current (pState->current)
#define pState_ahead   (pState->ahead)

#define is_op(LexTokenType)                                   (is_in(LexTokenType, valid_Operations, sizeof(valid_Operations)/sizeof(LexTokenEnum)))
#define is_EOF_or_end(LexTokenType)                           (LexTokenType == TK_EOF || LexTokenType == TK_end)
#define is_function_end(LexTokenType)                         (token_to_signify_end == TK_COMMA && (LexTokenType == TK_PARENTHESIS_R || LexTokenType == TK_COMMA))
#define is_valid_Expr_end(LexTokenType, current_tk_type)      (is_EOF_or_end(LexTokenType) || current_tk_type == LexTokenType)

static ExpressionAST* eval_expression_parser(ParseState* pState, LexTokenEnum token_to_signify_end, bool is_global_scope);

static ExpressionAST* get_value_expression_parser(ParseState* pState, LexTokenEnum token_to_signify_end, bool is_global_scope)
{
    if (!pState || !token_to_signify_end) return NULL;

    ExpressionAST* exprAST = init_ExpressionAST_ptr();
    if (!exprAST) return NULL;
    exprAST->top = NULL;
    
    ExpressionNodeAST** current = &exprAST->top; // current exprNode we are on

    // PLAN:
    // WHEN REACHES VALID POINT IT ASSIGNS
    // ELSE EXITS
    // BUT I NEED TO MAINTAIN THAT IT CHANGES THE exprAST correctly when needs
    // but doesnt when it reaches a non continuation point

    bool dont_break  = true;
    bool on_negative = false; 
    while (true) {
        switch (pState_current.type)
        {
            case TK_nil:
                assign_ExpressionNodeAST(current, EXPRNODE_NIL);
                dont_break = false;
                break;

            case TK_Int_val:
                assign_ExpressionNodeAST(current, EXPRNODE_INT);
                (*current)->data.integer = pState_current.integer;
                dont_break = false;
                break;

            case TK_Bool_val:
                assign_ExpressionNodeAST(current, EXPRNODE_BOOL);
                (*current)->data.bl = pState_current.bl;
                dont_break = false;
                break;
            
            case TK_SUB: // AUTO ASSUME ITS AT THE START OF A CHAIN OF NEG
                on_negative = !on_negative;
                break;

            case TK_Number_val:
                assign_ExpressionNodeAST(current, EXPRNODE_NUMBER);
                (*current)->data.number = pState_current.number;
                dont_break = false;
                break;

            case TK_Identifier:
                assign_ExpressionNodeAST(current, EXPRNODE_IDENTIFIER);
                (*current)->data.string_identifier = copystring(&pState_current.string);
                dont_break = false;
                break;

            case TK_String_val:
                assign_ExpressionNodeAST(current, EXPRNODE_STRING);
                (*current)->data.string_identifier = copystring(&pState_current.string);
                dont_break = false;
                break;

            case TK_PARENTHESIS_L:
                destroy_ExpressionAST_ptr(&exprAST);
                G_log_push_layer();
                exprAST = eval_expression_parser(pState, TK_PARENTHESIS_R, is_global_scope);
                G_log_pop_layer();
                
                if (!exprAST)      return NULL;    // failed alloc
                if (exprAST->fail) return exprAST; // pass the state over

                ExpressionNodeAST* tmp = exprAST->top;
                exprAST->top = NULL;

                assign_ExpressionNodeAST(&exprAST->top, EXPRNODE_PARENTHESIS);
                exprAST->top->right = tmp;

                dont_break = false;
                advance_parser(pState);
                break;

            case TK_UNKNOWN:
                destroy_ExpressionNodeAST_ptr(&exprAST->top);
                exprAST->fail = true;
                pState->errmsg = pState->lState.errmsg;
                dont_break = false;
                break;

            case TK_ERR:
                destroy_ExpressionNodeAST_ptr(&exprAST->top);
                exprAST->fail = true;
                pState->errmsg = pState->lState.errmsg;
                dont_break = false;
                break;

            default:
                G_log("got no value (%s)\n", LexTokenEnum_to_string(pState_current.type));
                dont_break = false;
                break;
        }
        if (!dont_break) break;
        advance_parser(pState);
    }

    if (on_negative) {
        ExpressionNodeAST* tmp = NULL;
        assign_ExpressionNodeAST(&tmp, EXPRNODE_NEG);
        tmp->right = exprAST->top;
        exprAST->top = tmp;
    }

    return exprAST;
}


// IF YOU ONLY RECIEVE NULL, THAT MEANS IT FAILED TO ALLOC MEM
// ELSE TO KNOW IF ITD FAIL, YOU'D CHECK ...->failed
// EXPECT IF IT ->failed THAT ->top IS COMPLETELY INVALID
static ExpressionAST* eval_expression_parser_section(ParseState* pState, LexTokenEnum token_to_signify_end, bool is_global_scope)
{
    ExpressionAST* exprAST = init_ExpressionAST_ptr();
    if (!exprAST) return NULL;

    advance_parser(pState);

    G_log_push_layer();
    ExpressionAST* value = get_value_expression_parser(pState, token_to_signify_end, is_global_scope);
    G_log_pop_layer();
    { // checking the status of the current value, whether it exists or not or whatever
        if (!value) {
            G_log("invalid! (value = NULL)\n");
            exprAST->fail = true;
            return exprAST; // this one is required to return asap

        } else if (value->fail) {
            G_log("invalid! (it failed...)\n");
            //pState->errmsg = "Expression was not finished!";
            exprAST->fail = true;
            destroy_ExpressionAST_ptr(&value);
            return exprAST;

        } else if (!value->top) {
            G_log("value doesnt exist!\n");
            destroy_ExpressionAST_ptr(&value);
            exprAST->fail = true;
            return exprAST;
        }
    }
    
    { // Checking if its not just negative symbols like this: <val> + -- (nothing after) 
        ExpressionNodeAST* check = value->top;

        while (check) {
            if (check->type == EXPRNODE_NEG) check = check->right;
            else break;
        }
        if (!check) {
            G_log("check doesnt exist so its invalid!\n");
            pState->errmsg = "Expression was not finished!";
            exprAST->fail = true;
            destroy_ExpressionNodeAST_ptr(&exprAST->top);
            destroy_ExpressionAST_ptr(&value);
            return exprAST;
        }
    }

    if (is_op(pState_ahead.type)) {
        advance_parser(pState);

        { // merge the value and exprAST variables, making value invalid
            assign_ExpressionNodeAST(&exprAST->top, LexTokenEnum_to_ValidExpressionNodeType_Operation(pState_current.type));
            exprAST->top->left = value->top;

            value->top = NULL;
            destroy_ExpressionAST_ptr(&value);
        }
    } else {
        exprAST->top = value->top;
        value->top = NULL;
        destroy_ExpressionAST_ptr(&value);

        if (is_valid_Expr_end(token_to_signify_end, pState->ahead.type)) {
            return exprAST;
        }

        pState->errmsg = "Expected a valid end of the expression!";
        destroy_ExpressionNodeAST_ptr(&exprAST->top);
        exprAST->fail = true;
        advance_parser(pState);
    }
    return exprAST;
}


static ExpressionAST* eval_expression_parser(ParseState* pState, LexTokenEnum token_to_signify_end, bool is_global_scope)
{
    ExpressionAST* main = eval_expression_parser_section(pState, token_to_signify_end, is_global_scope);
    ExpressionNodeAST** current = NULL; // POINTS TO THE CURRENT main BRANCH | IF CURRENT == NULL IT MEANS WE HAVE REACHED THE END OF THE PARSER

    if (!main)
        return NULL;

    if (main->fail)
        return main;

    if (get_Precedence_level(main->top->type) == 0)
        return main;

    current = &main->top;

    do { // IF CURRENT == NULL IT MEANS WE HAVE REACHED THE END OF THE PARSER
        ExpressionAST* right;
        
        { // fetching right
            right = eval_expression_parser_section(pState, token_to_signify_end, is_global_scope);

            if (!right) {
                destroy_ExpressionAST_ptr(&right);
                destroy_ExpressionNodeAST_ptr(&main->top);
                main->fail = true;
                return main;
            }
            if (!right->top || right->fail) {
                destroy_ExpressionAST_ptr(&right);
                destroy_ExpressionNodeAST_ptr(&main->top);
                main->fail = true;
                return main;
            }
        }

        ExpressionNodeAST* c_ref = *current;
        G_byte current_precedence  = get_Precedence_level(c_ref->type);
        G_byte right_precedence    = get_Precedence_level(right->top->type);

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
        destroy_ExpressionAST_ptr(&right);
    } while (current);

    return main;
}


ASTDatatype get_datatype_parser(ParseState* pState)
{
    advance_parser(pState);
    
    switch (pState->current.type)
    {
        case TK_char:       return ASTDATATYPE_CHAR;
        case TK_string:     return ASTDATATYPE_STRING;
        case TK_int:        return ASTDATATYPE_INT;
        case TK_number:     return ASTDATATYPE_NUMBER;
        case TK_bool:       return ASTDATATYPE_BOOL;
        case TK_function:   return ASTDATATYPE_FUNCTION;
        case TK_dynamic:    return ASTDATATYPE_DYNAMIC;
        default:            return ASTDATATYPE_ERR;
    }
}



// DECLARATION/ASSIGNMENT
G_AST eval_variable_parser(ParseState* pState, LexTokenEnum ending, bool is_global_scope)
{
    advance_parser(pState);

    G_AST ASTNode = {0};
    ASTNode.nodetype = ASTNODE_DECLARATION;
    ASTNode.error = false;

    if (pState_current.type != TK_Identifier) {
        pState->errmsg = "Expected an identifier...";
        ASTNode.error = true;
        return ASTNode;
    }
    ASTNode.declarationAST.info.identifier = copystring(&pState_current.string);
    G_log("-------------------------------------------------------\n");
    G_log("done copying string %s\n", ASTNode.declarationAST.info.identifier.content);

    if (get_var_info(pState, ASTNode.declarationAST.info.identifier.content)) {
        pState->errmsg = "Identifier name already exists!";
        ASTNode.error = true;
        return ASTNode;
    }

    G_log("PARSING DECLARATION FOR [%s]\n", ASTNode.declarationAST.info.identifier.content);

    advance_parser(pState);

    if (pState_current.type != TK_COLON) {
        pState->errmsg = "Expected a colon...";
        ASTNode.error = true;
        return ASTNode;
    }

    ASTNode.declarationAST.info.datatype = get_datatype_parser(pState);
    if (!ASTNode.declarationAST.info.datatype) {
        pState->errmsg = "Invalid datatype!";
        ASTNode.error = true;
        return ASTNode;
    }

    if (pState_ahead.type != TK_ASSIGN) {
        advance_parser(pState); // just for the error
        pState->errmsg = "Expected '='!";
        ASTNode.error = true;
        return ASTNode;
    }
    advance_parser(pState);

    G_log("parsing...\n");
    G_log_push_layer();
    ASTNode.declarationAST.expression = eval_expression_parser(pState, ending, is_global_scope);
    G_log_pop_layer();

    {
        G_log("Expression tree:\n");
        G_log("-------------------\n");
        G_log_push_layer();
        G_log_ExpressionNodeAST(ASTNode.declarationAST.expression->top);
        G_log_pop_layer();
        G_log("-------------------\n");
    }
    G_log("determining if its invalid\n");
    if (!ASTNode.declarationAST.expression->top) {
        ASTNode.error = true;
        if (!pState->errmsg) pState->errmsg = "Expected an expression! (from declaration)";
    }
    else if (ASTNode.declarationAST.expression->fail) {
        ASTNode.error = true;
    }
    bool adding_var_status = add_ParseScopeNode_variable(pState, &ASTNode.declarationAST.info.identifier, ASTNode.declarationAST.info.datatype);
    if (adding_var_status) {
        G_log("done adding name\n");
        G_log("-------------------------------------------------------\n");
    } else {
        G_log("failed adding name!\n");
        G_log("-------------------------------------------------------\n");
        ASTNode.error = true;
        return ASTNode;
    }
    ASTNode.declarationAST.slot = pState->scope_top->var_info.arr[pState->scope_top->var_info.length-1].slot;
    return ASTNode;
}






static ASTScope parser_get_scope(ParseState* pState, const ScopeType scopetype, const LexTokenEnum ending)
{
    if (!pState) return (ASTScope){0};

    ASTScope scope = init_ASTScope();
    if (!scope.nodes) return (ASTScope){0};

    add_ParseScopeNode(pState, scopetype);

    // DO NOT AND I MEAN NOT DESTROY X
    G_AST x;
    while (true) {
        x = parse_segment(pState, ending, false);
        
        if (x.error) {
            destroy_ASTScope(&scope);
            break;
        }
        if (x.nodetype == ASTNODE_END)
            break;
        if (x.nodetype == ASTNODE_IGNORE)
            continue;

        add_G_AST_to_ASTScope(&scope, x);
    }

    G_log("add all existing variables to nuke\n");
    pop_ParseScopeNode(pState, &scope);
    G_log("return scope\n");
    return scope;
}







static G_AST eval_if_and_while_statement(ParseState* pState)
{
    if (!pState) return (G_AST){0};

    G_log("doing if/while statement\n");

    G_AST x = (G_AST){0};
    x.error = true;

    if (pState_current.type == TK_if) {
        x.nodetype = ASTNODE_IF;
    } 
    else if (pState_current.type == TK_while) {
        x.nodetype = ASTNODE_WHILE;
    }
    else {
        G_log("invalid current token for if/while!\n");
        return x;
    }

    G_log("-------------------------------------------------------\n");
    G_log_push_layer();
    x.ifWhileAST.expression = eval_expression_parser(pState, (x.nodetype == ASTNODE_IF)? TK_then : TK_do, false);
    G_log_pop_layer();
    G_log("-------------------------------------------------------\n");
    if (!x.ifWhileAST.expression) return x;
    if (x.ifWhileAST.expression->fail) {
        destroy_ExpressionAST_ptr(&x.ifWhileAST.expression);
        return x;
    }

    if (!x.ifWhileAST.expression->top) {
        pState->errmsg = "Expected an expression! (from if)";
        destroy_ExpressionAST_ptr(&x.ifWhileAST.expression);
        return x;
    }

    G_log("Statement:\n");
    G_log_ExpressionNodeAST(x.ifWhileAST.expression->top);

    advance_parser(pState);

    G_log("-------------------------------------------------------\n");
    G_log_push_layer();
    x.ifWhileAST.nodes = parser_get_scope(pState, (x.nodetype == ASTNODE_IF)? SCOPE_IF : SCOPE_WHILE, TK_end);
    G_log_pop_layer();
    G_log("-------------------------------------------------------\n");

    if (!x.ifWhileAST.nodes.nodes) {
        destroy_ExpressionAST_ptr(&x.ifWhileAST.expression);
        destroy_ASTScope(&x.ifWhileAST.nodes);
        return x;
    }

    x.error = false;
    return x;
}





static G_AST eval_scope_statement(ParseState* pState)
{
    if (!pState) return (G_AST){0};

    G_log("doing scope statement\n");

    G_AST x = (G_AST){0};
    x.nodetype = ASTNODE_SCOPE;
    x.error = true;

    G_log("-------------------------------------------------------\n");
    G_log_push_layer();
    x.scopeAST = parser_get_scope(pState, SCOPE_SCOPE, TK_end);
    G_log_pop_layer();
    G_log("-------------------------------------------------------\n");

    if (!x.scopeAST.nodes) {
        destroy_ASTScope(&x.scopeAST);
        return x;
    }

    x.error = false;
    return x;
}




G_AST parse_segment(ParseState* pState, const LexTokenEnum ending, const bool is_global_scope)
{
    advance_parser(pState);
    
    G_log("parsing a new segment\n");

    switch (pState_current.type)
    {
        case TK_var:
            return eval_variable_parser(pState, ending, is_global_scope);

        case TK_if:
            if (is_global_scope) {
                pState->errmsg = "Cannot use a local-only statement in the Global scope!";
                return (G_AST){
                    .error=true,
                    .nodetype=ASTNODE_IF
                };
            }
            
            G_log("doing if\n");
            return eval_if_and_while_statement(pState);

        case TK_while:
            if (is_global_scope) {
                pState->errmsg = "Cannot use a local-only statement in the Global scope!";
                return (G_AST){
                    .error=true,
                    .nodetype=ASTNODE_WHILE
                };
            }

            G_log("doing while\n");
            return eval_if_and_while_statement(pState);

        case TK_do:
            /*
            if (is_global_scope) {
                pState->errmsg = "Cannot use a local-only statement in the Global scope!";
                return (G_AST){
                    .error=true,
                    .nodetype=ASTNODE_SCOPE
                };
            }
                */
            return eval_scope_statement(pState);

        default: {
            if (pState_current.type == TK_SEMI_COLON && ending != TK_SEMI_COLON) {
                G_log("is semi\n");
                G_AST x = (G_AST){0};
                return x;
            }

            // to tell the IR to stop looping, the scope has closed or the end of the file
            if (pState_current.type == ending) {
                G_log("is ending\n");
                G_AST x = (G_AST){0};
                x.nodetype = ASTNODE_END;
                return x;
            }

            G_log("no matches!\n");
            pState->errmsg = "Expected a valid statement!";

            G_AST x = (G_AST){0};
            x.error = true;
            return x;
        }
    }
}

#endif