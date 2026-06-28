#ifndef LEXER_H
#define LEXER_H

#include "../../etc/strings.h"
#include "../../etc/declarations.h"
#include <inttypes.h>
#include <stdbool.h>

typedef enum {
    TK_UNINIT = 0,

    TK_Identifier,
    TK_SEMI_COLON,
    TK_COLON,
    TK_DOT,

    TK_var,

    // DATA
    TK_Int_val,
    TK_Number_val,
    TK_String_val,
    TK_Char_val,
    TK_Bool_val,
    TK_nil,
    TK_NaN,

    // CONTROL FLOW
    TK_if,
    TK_then,
    TK_else,
    TK_while,
    TK_for,
    TK_do,
    TK_end,

    TK_return,
    TK_continue,
    TK_break,

    // LOGIC
    TK_and,
    TK_or,
    TK_not,
    TK_EQU,
    TK_NOT_EQU,
    TK_GT,
    TK_LT,
    TK_GT_EQU,
    TK_LT_EQU,

    // DATATYPES
    TK_function,
    TK_int,
    TK_number,
    TK_char,
    TK_bool,
    TK_string,
    TK_dynamic,

    // MATH/OPERATIONS
    TK_ASSIGN,
    TK_ADD,
    TK_SUB,
    TK_MUL,
    TK_DIV,
    TK_MOD,
    TK_POW,
    TK_PARENTHESIS_L,
    TK_PARENTHESIS_R,
    TK_HASH,
 
    TK_BRACKET_L,
    TK_BRACKET_R,

    TK_CURLY_L,
    TK_CURLY_R,
    TK_COMMA,

    // MISC
    TK_EOF,
    TK_UNKNOWN,
    TK_ERR,
} LexTokenEnum;

typedef struct {
    LexTokenEnum type;
    size_t line;
    size_t column;
    size_t length;

    union {
        bool        bl;
        char        ch;
        String      string;
        G_int64     integer;
        G_number64  number;
    };
} LexToken;

typedef struct {
    char*    string;
    size_t   str_length;

    size_t   index;
    char*    errmsg;

    size_t   current_column;
    size_t   current_line;
} LexState;

char* LexTokenEnum_to_string(LexTokenEnum e);

LexState init_LexState(String* file_content);
LexToken advance_lexer(LexState* lState);
void     destroy_LexToken(LexToken* lToken);

#endif