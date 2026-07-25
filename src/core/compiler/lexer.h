#ifndef LEXER_H
#define LEXER_H

#include "../../etc/strings.h"
#include "../../../include/declarations.h"
#include <inttypes.h>
#include <stdbool.h>

typedef enum GUIN_LexTokenEnum {
    GUIN_TK_UNINIT = 0,

    GUIN_TK_Identifier,
    GUIN_TK_SEMI_COLON,
    GUIN_TK_COLON,
    GUIN_TK_DOT,

    GUIN_TK_var,

    // DATA
    GUIN_TK_Int_val,
    GUIN_TK_Number_val,
    GUIN_TK_String_val,
    GUIN_TK_Char_val,
    GUIN_TK_Bool_val,
    GUIN_TK_nil,
    GUIN_TK_NaN,

    // CONTROL FLOW
    GUIN_TK_if,
    GUIN_TK_then,
    GUIN_TK_else,
    GUIN_TK_while,
    GUIN_TK_for,
    GUIN_TK_do,
    GUIN_TK_end,

    GUIN_TK_return,
    GUIN_TK_continue,
    GUIN_TK_break,

    // LOGIC
    GUIN_TK_and,
    GUIN_TK_or,
    GUIN_TK_not,
    GUIN_TK_EQU,
    GUIN_TK_NOT_EQU,
    GUIN_TK_GT,
    GUIN_TK_LT,
    GUIN_TK_GT_EQU,
    GUIN_TK_LT_EQU,

    // DATATYPES
    GUIN_TK_func,
    GUIN_TK_int,
    GUIN_TK_number,
    GUIN_TK_char,
    GUIN_TK_bool,
    GUIN_TK_string,
    GUIN_TK_dynamic,
    GUIN_TK_void,

    // MATH/OPERATIONS
    GUIN_TK_ASSIGN,
    GUIN_TK_ADD,
    GUIN_TK_SUB,
    GUIN_TK_MUL,
    GUIN_TK_DIV,
    GUIN_TK_MOD,
    GUIN_TK_POW,
    GUIN_TK_PARENTHESIS_L,
    GUIN_TK_PARENTHESIS_R,
    GUIN_TK_HASH,
 
    GUIN_TK_BRACKET_L,
    GUIN_TK_BRACKET_R,

    GUIN_TK_CURLY_L,
    GUIN_TK_CURLY_R,
    GUIN_TK_COMMA,

    // MISC
    GUIN_TK_EOF,
    GUIN_TK_UNKNOWN,
    GUIN_TK_ERR,
} GUIN_LexTokenEnum;

typedef struct {
    GUIN_LexTokenEnum type;
    size_t line;
    size_t column;
    size_t length;

    union {
        bool        bl;
        char        ch;
        GUIN_String      string;
        GUIN_int64     integer;
        GUIN_number64  number;
    };
} GUIN_LexToken;

typedef struct {
    char*    string;
    size_t   str_length;

    size_t   index;
    char*    errmsg;

    size_t   current_column;
    size_t   current_line;
} GUIN_LexState;

char* GUIN_LexTokenEnum_to_string(GUIN_LexTokenEnum e);

GUIN_LexState GUIN_init_LexState(GUIN_String* file_content);
GUIN_LexToken GUIN_advance_lexer(GUIN_LexState* lState);
void          GUIN_destroy_LexToken(GUIN_LexToken* lToken);

#endif
