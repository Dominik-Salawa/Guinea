#ifndef LEXER_H
#define LEXER_H

#include "../../etc/strings.h"
#include "../../../include/declarations.h"
#include <inttypes.h>
#include <stdbool.h>

typedef enum guin_LexTokenEnum {
    guin_TK_UNINIT = 0,

    guin_TK_Identifier,
    guin_TK_SEMI_COLON,
    guin_TK_COLON,
    guin_TK_DOT,

    guin_TK_var,

    // DATA
    guin_TK_Int_val,
    guin_TK_Number_val,
    guin_TK_String_val,
    guin_TK_Char_val,
    guin_TK_Bool_val,
    guin_TK_nil,
    guin_TK_NaN,
    guin_TK_Inf,

    // CONTROL FLOW
    guin_TK_if,
    guin_TK_then,
    guin_TK_else,
    guin_TK_while,
    guin_TK_for,
    guin_TK_do,
    guin_TK_end,

    guin_TK_return,
    guin_TK_continue,
    guin_TK_break,

    guin_TK_namespace,
    guin_TK_class,

    // DATATYPES
    guin_TK_func,
    guin_TK_int,
    guin_TK_number,
    guin_TK_char,
    guin_TK_bool,
    guin_TK_string,
    guin_TK_dynamic,
    guin_TK_void,


// FOR ADDING OPERATIONS
// PLEASE UPDATE guin_is_op
// IN parser.h
    // OPERATIONS
	   // arithmetic
    guin_TK_ASSIGN,
    guin_TK_ADD,
    guin_TK_SUB,
    guin_TK_MUL,
    guin_TK_DIV,
    guin_TK_MOD,
    guin_TK_POW,
	   // logic
	guin_TK_and,
    guin_TK_or,
    guin_TK_not,
    guin_TK_EQU,
    guin_TK_NOT_EQU,
    guin_TK_GT,
    guin_TK_LT,
    guin_TK_GT_EQU,
    guin_TK_LT_EQU,

    guin_TK_PARENTHESIS_L,
    guin_TK_PARENTHESIS_R,
    guin_TK_HASH,
 
    guin_TK_BRACKET_L,
    guin_TK_BRACKET_R,

    guin_TK_CURLY_L,
    guin_TK_CURLY_R,
    guin_TK_COMMA,
	guin_TK_MACRO_ARG_SPLIT,

    // MISC
    guin_TK_EOF,
    guin_TK_UNKNOWN,
    guin_TK_ERR,
} guin_LexTokenEnum;

typedef struct {
    guin_LexTokenEnum type;
    size_t line;
    size_t column;
    size_t length;

    union {
        bool        bl;
        char        ch;
        guin_String      string;
        guin_int64     integer;
        guin_number64  number;
    };
} guin_LexToken;

typedef struct {
    char*    string;
    size_t   str_length;

    size_t   index;
    char*    errmsg;

    size_t   current_column;
    size_t   current_line;
} guin_LexState;

char* guin_LexTokenEnum_to_string(guin_LexTokenEnum e);

bool guin_init_LexState(guin_LexState* lState, guin_String* file_content);
guin_LexToken guin_advance_lexer(guin_LexState* lState);
void          guin_destroy_LexToken(guin_LexToken* lToken);
void guin_destroy_LexState(guin_LexState* x);

#endif
