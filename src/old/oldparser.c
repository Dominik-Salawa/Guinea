#ifndef PARSER_C
#define PARSER_C

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../etc/strings.h"
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

LexTokenEnum valid_Datatypes[] = {
    TK_Identifier,
    TK_int,
    TK_number,
    TK_bool,
    TK_string,
    TK_char,
    TK_function
};

LexTokenEnum valid_Operations[] = {
    TK_ADD,
    TK_SUB,
    TK_MUL,
    TK_DIV,
    TK_MOD,
    TK_POW,

    TK_and,
    TK_or,

    TK_GT,
    TK_LT,
    TK_EQU,
    TK_NOT_EQU,
    TK_GT_EQU,
    TK_LT_EQU,
};

bool is_in(LexTokenEnum x, LexTokenEnum* array, size_t len) 
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



// IMPORTANT THINGS

// IF token_to_signify_end == TK_EOF/TK_end THEN itll also allow semi-colon
bool eval_expression_parser(ParseState* pState, LexTokenEnum token_to_signify_end, bool is_global_scope)
{
    advance_parser(pState);
    if (pState->current.type == token_to_signify_end && !(is_function_end(pState_current.type) && pState_current.type == TK_PARENTHESIS_R)) {
        pState->errmsg = "Expression was not finished!";
        return false;
    }
    // if its TK_EOF
    if ((is_semi_colon_approved(token_to_signify_end) && pState_current.type == TK_SEMI_COLON) && !(is_function_end(pState_current.type) && pState_current.type == TK_PARENTHESIS_R)) {
        pState->errmsg = "Expression was not finished!";
        return false;
    }

    bool negative = false;
    bool supposed_to_be_on_value = true;

    while (true) {
        if (pState_current.type == token_to_signify_end) {
            printf("%s ", LexTokenEnum_to_string(pState_current.type));
            break;
        }

        if ((is_semi_colon_approved(token_to_signify_end) && pState_current.type == TK_SEMI_COLON) || is_function_end(pState_current.type)) {
            printf("%s ", LexTokenEnum_to_string(pState_current.type));
            break;
        }

        if (pState_current.type == TK_SUB && supposed_to_be_on_value) {
            advance_parser(pState);
            negative = true;

            if (pState_current.type != TK_Number_val && 
                pState_current.type != TK_Int_val && 
                pState_current.type != TK_Identifier && 
                pState_current.type != TK_Bool_val && 
                pState_current.type != TK_PARENTHESIS_L) {
                pState->errmsg = "Tried to make a value negative which cannot be negative!";
                return false;
            }

            printf("-");
        }

        // PLS MAKE WORK AND MAKE SURE BOOLS WORK AS WELLLLLLLL

        if (pState_current.type == TK_Number_val) {
            printf("%f ", pState_current.number);
            supposed_to_be_on_value = false;
        }
        else if (pState_current.type == TK_Int_val) {
            printf("%lld ", pState_current.integer);
            supposed_to_be_on_value = false;
        }
        else if (pState_current.type == TK_Identifier) {
            printf("%s ", pState_current.string.content);
            supposed_to_be_on_value = false;

            if (pState_ahead.type == TK_PARENTHESIS_L) {
                advance_parser(pState);
                if (is_global_scope) {
                    pState->errmsg = "Cannot call functions in the global scope of an expression!";
                    return false;
                }
                bool state = get_function_args_in_expression_parser(pState);
                if (!state) return false;
            }
        }
        else if (pState_current.type == TK_PARENTHESIS_L) {
            printf("(");
            if (!eval_expression_parser(pState, TK_PARENTHESIS_R, is_global_scope)) {
                return false;
            }
            supposed_to_be_on_value = false;
        }
        else if (pState_current.type == TK_Bool_val) {
            printf((pState_current.bl) ? "true " : "false ");
            supposed_to_be_on_value = false;
        }
        else if (pState_current.type == TK_String_val) {
            printf("\"%s\" ", pState_current.string.content);
            supposed_to_be_on_value = false;
        }
        else if (pState_current.type == TK_Char_val) {
            printf("'%c' ", pState_current.ch);
        }
        else if (is_op(pState_current.type)) {
            if (!supposed_to_be_on_value) {
                printf("%s ", LexTokenEnum_to_string(pState_current.type));
                supposed_to_be_on_value = true;
            } else {
                pState->errmsg = "Expected a value but gotten an operation/token!";
                return false;
            }
        } else {
            pState->errmsg = "Expected a value but gotten an operation/token!";
            return false;
        }

        if (!supposed_to_be_on_value) {
            if (is_semi_colon_approved(token_to_signify_end)) {
                if (!is_op(pState_ahead.type)) {
                    break;
                }
            } else if (pState_ahead.type != token_to_signify_end) {
                if (!is_op(pState_ahead.type) && !is_function_end(pState_ahead.type)) {
                    pState->errmsg = "Expected the expression to continue!";
                    return false;
                }
            }
        }

        advance_parser(pState);
    }

    return true;
}

bool get_function_args_in_expression_parser(ParseState* pState) // FIX PLEASE
{
    bool first_iteration = true;
    
    /*
    printf("Parser log: `%s` [`%s` - %s~%s~%s]\n", pState->errmsg, pState_current.string.content, 
        LexTokenEnum_to_string(pState_prev.type), 
        LexTokenEnum_to_string(pState_current.type),
        LexTokenEnum_to_string(pState_ahead.type)
    );
    */
    
    printf("(");

    //printf("MAKE TRAILING COMMAS NOT WORK AND PROPERLY SECURE THE FUNCTION ARGS PLEESEAWDAWDAWDFAWFGLKQAJNFWOLKAWNGKJ\n");

    //printf("Parser log: `%s` [`%s` - %s~%s~%s]\n", pState->errmsg, pState_current.string.content, 
    //    LexTokenEnum_to_string(pState_prev.type), 
    //    LexTokenEnum_to_string(pState_current.type),
    //    LexTokenEnum_to_string(pState_ahead.type)
    //);

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

            bool state = eval_expression_parser(pState, TK_COMMA, false);
            if (!state) return false;
            first_iteration = false;
        }
    }

    return true;
}







// DECLARATION/ASSIGNMENT

bool eval_variable_parser(ParseState* pState, LexTokenEnum ending, bool is_global_scope)
{
    printf("is var\n");
    advance_parser(pState);

    if (pState_current.type != TK_Identifier) {
        pState->errmsg = "Expected an identifier...";
        return false;
    }

    
    String identifier = copystring(&pState_current.string);
    advance_parser(pState);

    if (pState_current.type != TK_COLON) {
        pState->errmsg = "Expected a colon...";
        return false;
    }
    advance_parser(pState);

    ASTDatatype datatype = get_datatype_parser();
    if (!datatype) {
        pState->errmsg = "Invalid datatype!";
        return false;
    }
    advance_parser(pState);

    if (is_semi_colon_approved(ending)) {
        if (pState_current.type != TK_ASSIGN)
            return true;
    }

    if (pState_current.type == ending) {
        return true;
    }

    if (pState_current.type != TK_ASSIGN) {
        pState->errmsg = "Expected '='!";
        return false;
    }

    printf("Identifier DECLARE `%s`:\n\t", identifier.content);
    clearstring(&identifier);
    return eval_expression_parser(pState, ending, is_global_scope);
}


bool eval_identifier(ParseState* pState, LexTokenEnum ending, bool is_global_scope)
{
    String identifier = copystring(&pState_current.string);
    advance_parser(pState);

    printf("%s\n", LexTokenEnum_to_string(pState_current.type));

    bool status = false;

    if (pState_current.type == TK_PARENTHESIS_L) {
        printf("Identifier CALL `%s`:\n\t", identifier.content);
        status = get_function_args_in_expression_parser(pState);
        printf("done\n");
    } else if (pState_current.type == TK_ASSIGN) {
        printf("Identifier ASSIGN `%s`:\n\t", identifier.content);
        status = eval_expression_parser(pState, ending, is_global_scope);
    } else {
        pState->errmsg = "Expected '='!";
        return false;
    }

    clearstring(&identifier);
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


bool eval_for_statement(ParseState* pState)
{
    // first segment
    bool status = parse_segment(pState, TK_SEMI_COLON, false);
    if (pState->errmsg) return false;

    // second segment
    status = eval_expression_parser(pState, TK_SEMI_COLON, false);
    if (pState->errmsg) return false;

    // third segment
    status = parse_segment(pState, TK_do, false);
    if (pState->errmsg) return false;

    printf("Attempting for loop now...\n");

    while (true) {
        status = parse_segment(pState, TK_end, false);
        if (pState->errmsg) return false;
        if (!status) break;
    }
    printf("BREAK! FOR\n");
    return true;
}



bool parse_segment(ParseState* pState, const LexTokenEnum ending, bool is_global_scope)
{
    advance_parser(pState);
    printf("ATTEMPTING: %s\n", LexTokenEnum_to_string(pState_current.type));
    
    switch (pState_current.type) 
    {
        case TK_var:             {  return eval_variable_parser(pState, ending, is_global_scope);    }
        case TK_if:              {  return eval_if_statement(pState);                                }
        case TK_while:           {  return eval_while_statement(pState);                             }
        case TK_for:             {  return eval_for_statement(pState);                               }
        case TK_Identifier:      {  return eval_identifier(pState, ending, is_global_scope);         }
        case TK_function:        {  return eval_function(pState);                                    }
    }

    if (is_semi_colon_approved(ending) && pState_current.type == TK_SEMI_COLON)
        return true;

    if (pState_current.type == ending) {
        printf("Break!\n");
        return false;
    }

    printf("no matches!\n");
    pState->errmsg = "Expected a statement!";
    return false;
}



bool parse(String* file_content)
{
    ParseState pState = init_ParseState(file_content);

    while (true) {
        printf("attempting...\n");
        if (!parse_segment(&pState, TK_EOF, true)) break;
        printf("cleared\n");
    }

    printf("Parser log: `%s` [`%s` - %s~%s~%s]\n", pState.errmsg, pState.current.string.content, 
        LexTokenEnum_to_string(pState.prev.type), 
        LexTokenEnum_to_string(pState.current.type),
        LexTokenEnum_to_string(pState.ahead.type)
    );

    destroy_ParseState(&pState);
}

#endif