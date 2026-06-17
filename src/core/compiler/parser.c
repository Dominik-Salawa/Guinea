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

ParseScopeNode init_ParseScopeNode(ScopeType scopetype)
{
    ParseScopeNode x = {0};
    x.scopetype = scopetype;
    return x;
}

void destroy_ParseScopeNode(ParseScopeNode** pScope)
{
    for (size_t i = 0; i < (*pScope)->var_info.var_info_len; i++)
        destroy_VariableInfoAST(&(*pScope)->var_info.arr[i]);

    free((*pScope)->var_info.arr);
    free(*pScope);
    *pScope = NULL;
}

bool add_ParseScopeNode(ParseState* pState, ScopeType scopetype)
{
    ParseScopeNode* x = malloc(sizeof(ParseScopeNode));
    if (!x) return false;
    *x = init_ParseScopeNode(scopetype);
    return true;
}

bool pop_ParseScopeNode(ParseState* pState)
{
    ParseScopeNode* x = pState->scope_top;
    if (!x) return false;
    pState->scope_top = x->prev;
    destroy_ParseScopeNode(&x);
    return true;
}
















ParseState init_ParseState(String* file_content)
{
    ParseState x = {0};
    x.lState = init_LexState(file_content);
    return x;
}

void destroy_ParseState(ParseState* pState)
{
    ParseScopeNode* top = pState->scope_top;

    while (top) {
        ParseScopeNode* next = top->prev;
        destroy_ParseScopeNode(&next);
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
    byte rank;
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
static byte get_Precedence_level(ExpressionNodeType type)
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

#define is_op(LexTokenType)                  (is_in(LexTokenType, valid_Operations, sizeof(valid_Operations)/sizeof(LexTokenEnum)))
#define is_semi_colon_approved(LexTokenType) (LexTokenType == TK_EOF || LexTokenType == TK_end)
#define is_function_end(LexTokenType)        (token_to_signify_end == TK_COMMA && (LexTokenType == TK_PARENTHESIS_R || LexTokenType == TK_COMMA))


static ExpressionAST* get_value_expression_parser(ParseState* pState, LexTokenEnum token_to_signify_end, bool is_global_scope)
{
    if (!pState || !token_to_signify_end) return NULL;

    ExpressionAST* exprAST = init_ExpressionAST_ptr();
    if (!exprAST) return NULL;
    exprAST->top = NULL;

    if (pState->current.type == token_to_signify_end && !(is_function_end(pState_current.type) && pState_current.type == TK_PARENTHESIS_R)) {
        return exprAST;
    }
    // if its TK_EOF
    if ((is_semi_colon_approved(token_to_signify_end) && pState_current.type == TK_SEMI_COLON) && !(is_function_end(pState_current.type) && pState_current.type == TK_PARENTHESIS_R)) {
        return exprAST;
    }

    ExpressionNodeAST** current = &exprAST->top; // current exprNode we are on
    G_log("current: %p\n", current);


    // PLAN:
    // WHEN REACHES VALID POINT IT ASSIGNS
    // ELSE EXITS
    // BUT I NEED TO MAINTAIN THAT IT CHANGES THE exprAST correctly when needs
    // but doesnt when it reaches a non continuation point

    while (true) {
        G_log("fetching value...\n");
        switch (pState_current.type)
        {
            case TK_nil:
                G_log("nil\n");
                assign_ExpressionNodeAST(current, EXPRNODE_NIL);
                return exprAST;

            case TK_Int_val:
                G_log("int\n");
                assign_ExpressionNodeAST(current, EXPRNODE_INT);
                (*current)->data.integer = pState_current.integer;
                return exprAST;

            
            case TK_SUB: // AUTO ASSUME ITS AT THE START OF A CHAIN OF NEG
                assign_ExpressionNodeAST(current, EXPRNODE_NEG);
                current = &((*current)->right);
                break;

            case TK_Number_val:
                G_log("number\n");
                assign_ExpressionNodeAST(current, EXPRNODE_NUMBER);
                (*current)->data.number = pState_current.number;
                return exprAST;

            /*
            case TK_Identifier:
                G_log("identifier");
                String identifier = copystring(&pState_current.string);
            
                 CHECK AHEAD IF ITS LIKE A FUNCTION CALL OR WHATEVER
                switch (pState_ahead.type)
                {
                    case TK_PARENTHESIS_L:
                        get_function_args_in_expression_parser(pState);
                        waddawdw;
                }
                

                assign_ExpressionNodeAST(current, EXPRNODE_IDENTIFIER);
                (*current)->data.string_identifier = identifier;
                return exprAST;
            */

            case TK_String_val:
                G_log("string\n");
                assign_ExpressionNodeAST(current, EXPRNODE_STRING);
                (*current)->data.string_identifier = copystring(&pState_current.string);
                return exprAST;

            default: 
                return exprAST;
        }
        advance_parser(pState);
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

    G_log("getting original value...\n");
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
            G_log("done destroying!\n");
            return exprAST;

        } else if (!value->top) {
            G_log("value doesnt exist!");
            destroy_ExpressionAST_ptr(&value);
            G_log("done destroying!\n");
            return exprAST;
        }
    }
    
    { // Checking if its not just negative symbols like this: <val> + -- (nothing after) 
        ExpressionNodeAST* check = value->top;

        G_log("checking if its just invalid parse tree slop... (if its just negs or logic not)\n");
        while (check) {
            if (check->type == EXPRNODE_NEG) check = check->right;
            else break;
        }
        G_log("done\n");
        if (!check) {
            G_log("check doesnt exist so its invalid!\n");
            pState->errmsg = "Expression was not finished!";
            exprAST->fail = true;
            destroy_ExpressionNodeAST_ptr(&exprAST->top);
            destroy_ExpressionAST_ptr(&value);
            G_log("done destroying\n");
            return exprAST;
        }

        { // Finish expressions like "-5"/"---5" so the number itself is negative
            G_log("FINISH ME FOR COLLAPSING INTS/NUMS DOWN INTO NEGATIVES AUTOMATICALLY!!!!\n");
        }
    }

    G_log("\tCurrent token: %s~%s~%s\n", LexTokenEnum_to_string(pState_prev.type), LexTokenEnum_to_string(pState_current.type), LexTokenEnum_to_string(pState_ahead.type));
    G_log("checking...\n");
    if (is_op(pState_ahead.type)) {
        G_log("is op!\n");
        advance_parser(pState);

        { // merge the value and exprAST variables, making value invalid
            assign_ExpressionNodeAST(&exprAST->top, LexTokenEnum_to_ValidExpressionNodeType_Operation(pState_current.type));
            exprAST->top->left = value->top;

            value->top = NULL;
            destroy_ExpressionAST_ptr(&value);
        }
        G_log("merged value and exprAST\n");
        G_log("CUR:%s\n", ExpressionNodeType_to_string(exprAST->top->type));
    } else {
        G_log("not an operation!\n");
        exprAST->top = value->top;
        value->top = NULL;
        destroy_ExpressionAST_ptr(&value);
    }

    G_log("\tCurrent token: %s~%s~%s\n", LexTokenEnum_to_string(pState_prev.type), LexTokenEnum_to_string(pState_current.type), LexTokenEnum_to_string(pState_ahead.type));
    G_log("RET:%s\n", ExpressionNodeType_to_string(exprAST->top->type));
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
            G_log("--------------\n");
            G_log_push_layer();
            right = eval_expression_parser_section(pState, token_to_signify_end, is_global_scope);
            G_log_pop_layer();
            G_log("--------------\n");

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
        byte current_precedence  = get_Precedence_level(c_ref->type);
        byte right_precedence    = get_Precedence_level(right->top->type);

        if (current_precedence >= right_precedence && right_precedence != 0) {
            G_log("current >= right && right != 0\n");

            // we position main to be on the left side of the lesser/equ right
            c_ref->right     = right->top->left;
            right->top->left = c_ref;
            *current = right->top; // resets the current pos where right->top is isntead of c_ref
        }
        else if (right_precedence == 0) {
            G_log("right == 0\n");
            c_ref->right = right->top;
            current = NULL;
        }
        else { // current_precedence < right_precedence
            G_log("current < right\n");
            c_ref->right = right->top;
            current = &c_ref->right;
        }
        if (current)
            G_log("current: %s\n", ExpressionNodeType_to_string((*current)->type));

        // before it exits
        right->top = NULL;
        destroy_ExpressionAST_ptr(&right);
    } while (current);

    return main;
}

// IMPORTANT THINGS
// IF token_to_signify_end == TK_EOF/TK_end THEN itll also allow semi-colon

/*
ExpressionAST* get_function_args_in_expression_parser(ParseState* pState) // FIX PLEASE
{
    bool first_iteration = true;
        
    printf("(");

    ExpressionAST* functionArgs = {0};

    while (true) {
        if (pState_current.type == TK_PARENTHESIS_R) {
            break;
        } else {
            //printf("PARAM %s: ", LexTokenEnum_to_string(pState_ahead.type));

            if (pState_ahead.type == TK_COMMA) {
                pState->errmsg = "Unexpected comma!";
                return false;
            }
            if (pState_ahead.type == TK_PARENTHESIS_R && !first_iteration) {
                pState->errmsg = "Unfinished argument!";
                return false;
            }

            ExpressionAST expression = eval_expression_parser(pState, TK_COMMA, false);
            if (expression.fail) {
                return false;
            }
            first_iteration = false;
        }
    }

    return (ExpressionAST*){0};
}
*/

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

    G_log_ExpressionNodeAST(ASTNode.declarationAST.expression->top);
    putchar(10);
    G_log("determining if its invalid\n");
    if (!ASTNode.declarationAST.expression->top) {
        ASTNode.error = true;
        pState->errmsg = "Expected an expression!";
    }
    else if (ASTNode.declarationAST.expression->fail) {
        ASTNode.error = true;
    }
    G_log("-------------------------------------------------------\n");

    return ASTNode;
}


/*
bool eval_identifier(ParseState* pState, LexTokenEnum ending, bool is_global_scope)
{
    G_AST ASTNode;
    String identifier = copystring(&pState_current.string);
    advance_parser(pState);

    printf("%s\n", LexTokenEnum_to_string(pState_current.type));

    if (pState_current.type == TK_PARENTHESIS_L) {
        //printf("Identifier CALL `%s`:\n\t", identifier.content);
        status = get_function_args_in_expression_parser(pState);
        printf("done\n");
    } else if (pState_current.type == TK_ASSIGN) {
        //printf("Identifier ASSIGN `%s`:\n\t", identifier.content);
        ExpressionAST status = eval_expression_parser(pState, ending, is_global_scope);
        return status;
    } else {
        pState->errmsg = "Expected '='!";
        return false;
    }

    return status;
}



bool eval_get_function(ParseState* pState)
{
    printf("FINISH ME\n");
    return true;
}



bool eval_function(ParseState* pState)
{
    advance_parser(pState);

    if (pState_current.type != TK_Identifier) {
        pState->errmsg = "Expected 'identifier'!";
        return false;
    }

    String identifier = copystring(&pState_current.string);
    advance_parser(pState);

    if (pState_current.type != TK_PARENTHESIS_L) {
        pState->errmsg = "Expected '('!";
        return false;
    }

    printf("FINISH ME\n");
    return eval_get_function(pState);
}



// CONTROL FLOW

bool eval_if_statement(ParseState* pState)
{
    bool status = eval_expression_parser(pState, TK_then, false);
    if (!status) return false;

    while (true) {
        status = parse_segment(pState, TK_end, false);
        if (pState->errmsg) return false;
        if (!status) break;
    }
    printf("BREAK! IF\n");
    return true;
}


bool eval_while_statement(ParseState* pState)
{
    bool status = eval_expression_parser(pState, TK_do, false);
    if (!status) return false;

    while (true) {
        status = parse_segment(pState, TK_end, false);
        if (pState->errmsg) return false;
        if (!status) break;
    }
    printf("BREAK! WHILE\n");
    return true;
}



G_AST eval_for_statement(ParseState* pState)
{
    G_AST for_statement = {0};

    // first segment
    G_AST f_segment = parse_segment(pState, TK_SEMI_COLON, false);
    if (f_segment.nodetype == ASTNODE_ERROR) return status;

    // second segment
    status = eval_expression_parser(pState, TK_SEMI_COLON, false);
    if (pState->errmsg) return false;

    // third segment
    status = parse_segment(pState, TK_do, false);
    if (pState->errmsg) return false;

    while (true) {
        status = parse_segment(pState, TK_end, false);
        if (pState->errmsg) return false;
        if (!status) break;
    }
    printf("BREAK! FOR\n");
    return true;
}
*/



G_AST parse_segment(ParseState* pState, const LexTokenEnum ending, bool is_global_scope)
{
    advance_parser(pState);
    
    G_log("parsing a new segment\n");
    G_log_push_layer();

    switch (pState_current.type) 
    {
        case TK_var:             {  G_log_pop_layer(); return eval_variable_parser(pState, ending, is_global_scope);  }
        //case TK_if:              {  return eval_if_statement(pState);                              }
        //case TK_while:           {  return eval_while_statement(pState);                           }
        //case TK_for:             {  return eval_for_statement(pState);                             }
        //case TK_Identifier:      {  return eval_identifier(pState, ending, is_global_scope);       }
        //case TK_function:        {  return eval_function(pState);                                  }
        default: {
            G_log_pop_layer();

            if (is_semi_colon_approved(ending) && pState_current.type == TK_SEMI_COLON) {
                return (G_AST){0};
            }

            // to tell the IR to stop looping, the scope has closed or the end of the file
            if (pState_current.type == ending) {
                G_log("Break!\n");
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