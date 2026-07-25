#ifndef LEXER_C
#define LEXER_C

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "lexer.h"
#include "../../etc/string_manipulation.h"

char* GUIN_LexTokenEnum_to_string(GUIN_LexTokenEnum e)
{
    switch (e)
    {
        case GUIN_TK_Identifier:     return "identifier";
        case GUIN_TK_var:            return "var";

        case GUIN_TK_Int_val:        return "(int)";
        case GUIN_TK_Number_val:     return "(number)";
        case GUIN_TK_String_val:     return "(string)";
        case GUIN_TK_Char_val:       return "(char)";
        case GUIN_TK_Bool_val:       return "(bool)";
        case GUIN_TK_nil:            return "nil";
        case GUIN_TK_NaN:            return "NaN";

        case GUIN_TK_if:             return "if";
        case GUIN_TK_else:           return "else";
        case GUIN_TK_while:          return "while";
        case GUIN_TK_for:            return "for";
        case GUIN_TK_return:         return "return";
        case GUIN_TK_continue:       return "continue";
        case GUIN_TK_break:          return "break";

        case GUIN_TK_func:           return "func";
        case GUIN_TK_int:            return "int";
        case GUIN_TK_number:         return "number";
        case GUIN_TK_string:         return "string";
        case GUIN_TK_char:           return "char";
        case GUIN_TK_bool:           return "bool";
        case GUIN_TK_void:           return "void";

        case GUIN_TK_ASSIGN:         return "=";
        case GUIN_TK_ADD:            return "+";
        case GUIN_TK_SUB:            return "-";
        case GUIN_TK_MUL:            return "*";
        case GUIN_TK_DIV:            return "/";
        case GUIN_TK_MOD:            return "%";
        case GUIN_TK_POW:            return "^";

        case GUIN_TK_HASH:           return "#";

        case GUIN_TK_PARENTHESIS_L:  return "(";
        case GUIN_TK_PARENTHESIS_R:  return ")";
        case GUIN_TK_COLON:          return ":";
        case GUIN_TK_SEMI_COLON:     return ";";
        case GUIN_TK_DOT:            return ".";
        case GUIN_TK_COMMA:          return ",";

        case GUIN_TK_GT:             return ">";
        case GUIN_TK_LT:             return "<";
        case GUIN_TK_EQU:            return "==";
        case GUIN_TK_NOT_EQU:        return "!=";
        case GUIN_TK_LT_EQU:         return "<=";
        case GUIN_TK_GT_EQU:         return ">=";
        case GUIN_TK_and:            return "and";
        case GUIN_TK_or:             return "or";

        case GUIN_TK_CURLY_L:        return "{";
        case GUIN_TK_CURLY_R:        return "}";

        case GUIN_TK_BRACKET_L:      return "[";
        case GUIN_TK_BRACKET_R:      return "]";

        case GUIN_TK_EOF:            return "EOF";
        case GUIN_TK_UNINIT:         return "UNINIT";
        case GUIN_TK_UNKNOWN:        return "UNKNOWN";
        case GUIN_TK_ERR:            return "ERR";

        default:                return "(UNAVAILABLE)";
    }
}

struct GUIN_LexNodeStringToEnumData {
    char* string;
    GUIN_LexTokenEnum type;
};

struct GUIN_LexNodeCharToEnumData {
    char ch;
    GUIN_LexTokenEnum type;
};

static const struct GUIN_LexNodeStringToEnumData stringtoken[] = {
    {.string="true",        .type=GUIN_TK_Bool_val},
    {.string="false",       .type=GUIN_TK_Bool_val},
    {.string="nil",         .type=GUIN_TK_nil},
    {.string="NaN",         .type=GUIN_TK_NaN},
    {.string="void",        .type=GUIN_TK_void},

    {.string="func",        .type=GUIN_TK_func},
    {.string="int",         .type=GUIN_TK_int},
    {.string="number",      .type=GUIN_TK_number},
    {.string="char",        .type=GUIN_TK_char},
    {.string="bool",        .type=GUIN_TK_bool},
    {.string="string",      .type=GUIN_TK_string},

    {.string="if",          .type=GUIN_TK_if},
    {.string="else",        .type=GUIN_TK_else},
    {.string="while",       .type=GUIN_TK_while},
    {.string="for",         .type=GUIN_TK_for},
    {.string="not",         .type=GUIN_TK_not},
    {.string="and",         .type=GUIN_TK_and},
    {.string="or",          .type=GUIN_TK_or},

    {.string="var",         .type=GUIN_TK_var},
};

static const struct GUIN_LexNodeCharToEnumData chartoken[] = {
    {.ch='{',       .type=GUIN_TK_CURLY_L},
    {.ch='}',       .type=GUIN_TK_CURLY_R},
    {.ch='[',       .type=GUIN_TK_BRACKET_L},
    {.ch=']',       .type=GUIN_TK_BRACKET_R},

    {.ch='(',       .type=GUIN_TK_PARENTHESIS_L},
    {.ch=')',       .type=GUIN_TK_PARENTHESIS_R},
    {.ch='.',       .type=GUIN_TK_DOT},
    {.ch=':',       .type=GUIN_TK_COLON},
    {.ch=';',       .type=GUIN_TK_SEMI_COLON},
    {.ch=',',       .type=GUIN_TK_COMMA},

    {.ch='=',       .type=GUIN_TK_ASSIGN},
    {.ch='<',       .type=GUIN_TK_LT},
    {.ch='>',       .type=GUIN_TK_GT},
    {.ch='+',       .type=GUIN_TK_ADD},
    {.ch='-',       .type=GUIN_TK_SUB},
    {.ch='*',       .type=GUIN_TK_MUL},
    {.ch='/',       .type=GUIN_TK_DIV},
    {.ch='%',       .type=GUIN_TK_MOD},
    {.ch='^',       .type=GUIN_TK_POW},
    {.ch='#',       .type=GUIN_TK_HASH},
};

struct GUIN_DoubleOperationChar {
    char f_ch;
    char s_ch;
    GUIN_LexTokenEnum type;
};

static const struct GUIN_DoubleOperationChar doublechartoken[] = {
    {.f_ch='=', .s_ch='=',      .type=GUIN_TK_EQU},
    {.f_ch='!', .s_ch='=',      .type=GUIN_TK_NOT_EQU},
    {.f_ch='<', .s_ch='=',      .type=GUIN_TK_LT_EQU},
    {.f_ch='>', .s_ch='=',      .type=GUIN_TK_GT_EQU},
};

static void GUIN_set_token_type(GUIN_LexToken* token, GUIN_LexState* lState) // for true/false also converts into right datatype
{
    if (GUIN_is_int_string(token->string.content) || GUIN_is_valid_number(token->string.content)) {
        size_t decimal_index = 0;
        {
            bool found = false;

            for (size_t i = 0; i < token->string.length; i++) {
                if (token->string.content[i] == '.') { 
                    decimal_index = i;
                    found = true;
                    break; 
                }
            } 

            token->type = (found) ? GUIN_TK_Number_val : GUIN_TK_Int_val;
        }

        GUIN_int64     integer = 0;
        GUIN_number64  number  = 0;
        GUIN_number64  decimal = 0;

        if (token->type == GUIN_TK_Int_val) {
            for (size_t i = 0; i < token->string.length; i++) {
                integer = (integer * 10) + GUIN_to_number(token->string.content[i]);
            }
        } else {
            size_t i = 0;
            for (; i < decimal_index; i++) {
                number = (number * 10) + GUIN_to_number(token->string.content[i]);
            }
            size_t current_dec_pos = 1;
            for (i += 1; i < token->string.length; i++) {
                decimal = GUIN_to_number(token->string.content[i]);
                if (decimal != 0) decimal /= pow(10, current_dec_pos);
                number += decimal;
                ++current_dec_pos;
            }
        }

        GUIN_clearstring(&token->string);

        if (token->type == GUIN_TK_Number_val) {
            token->number  = number;
        } else { 
            token->integer = integer; 
        }

        return;
    }

    for (size_t i = 0; i < sizeof(stringtoken)/sizeof(struct GUIN_LexNodeStringToEnumData); i++) {
        if (strcmp(stringtoken[i].string, token->string.content) == 0) {
            GUIN_clearstring(&token->string);
            token->type = stringtoken[i].type;

            if (token->type == GUIN_TK_Bool_val)
                token->bl = stringtoken[i].string[0] == 't'; // if true, then 1 else 0

            else if (token->type == GUIN_TK_NaN) {
                token->type = GUIN_TK_Number_val;
                token->number = GUIN_NaN;
            }

            return;
        }
    }

    if (GUIN_is_valid_identifier_string(token->string.content)) {
        token->type = GUIN_TK_Identifier;
        return;
    }

    token->type = GUIN_TK_UNKNOWN;
    return;
}

struct GUIN_EscapeCodeToInt {
    char ch;
    char convert;
};

static const struct GUIN_EscapeCodeToInt escapeCodeToNumber[] = {
    {.ch='"',   .convert='"' },
    {.ch='\'',  .convert='\''},
    {.ch='t',   .convert='\t'},
    {.ch='n',   .convert='\n'},
    {.ch='v',   .convert='\v'},
    {.ch='r',   .convert='\r'},
    {.ch='a',   .convert='\a'},
    {.ch='b',   .convert='\b'},
    {.ch='f',   .convert='\f'},
    {.ch='\\',  .convert='\\'},
};

struct GUIN_ParseEscapeCode {
    char size;
    char ch;
    bool err;
};

static struct GUIN_ParseEscapeCode GUIN_parse_escape_code(char* str, size_t index)
{
    struct GUIN_ParseEscapeCode ret = {.ch=0,.err=false,.size=0};
    size_t len = 0;
    while (str[index+len] != 0) ++len;

    // IF ITS LIKE \n OR \t
    for (size_t i = 0; i < sizeof(escapeCodeToNumber)/sizeof(struct GUIN_EscapeCodeToInt); i++) {
        if (str[index] == escapeCodeToNumber[i].ch) {
            ret.ch = escapeCodeToNumber[i].convert;
            ret.size = 1;
            return ret;
        }
    }

    //
    // MAKE FUNCTION FOR GETTING OCTAL/HEX FOR REUSABILITY
    //

    // IF ITS OCTAL STATE
    if (GUIN_is_octal_char(str[index])) {
        for (; GUIN_is_octal_char(str[index+ret.size]) && ret.size < 3; ret.size++)
            ret.ch = (ret.ch * 8) + GUIN_to_number(str[index+ret.size]);

        if (ret.size > 0) return ret;
    }

    // for '\x' AND '\o'
    
    ++ret.size;
    switch (str[index]) {
        case 'x':
            for (; GUIN_is_hex_code(str[index+ret.size]) && ret.size < 3; ret.size++)
                ret.ch = (ret.ch * 16) + GUIN_to_hex_char(str[index+ret.size]);
            if (ret.size > 1) return ret;

        case 'o':
            for (; GUIN_is_octal_char(str[index+ret.size]) && ret.size < 4; ret.size++)
                ret.ch = (ret.ch * 8) + GUIN_to_number(str[index+ret.size]);
            if (ret.size > 1) return ret;
    }

    ret.size = 0;
    ret.err = true;
    return ret;
}

#define GUIN_lState_char       (lState->string[lState->index])
#define GUIN_ahead_lState_char (lState->string[lState->index + 1])

static void GUIN_increment_lexer(GUIN_LexState* lState, size_t incremental_size)
{
    lState->current_column += incremental_size;
    lState->index          += incremental_size;
}

static void GUIN_parse_lexer_string(GUIN_LexState* lState, GUIN_LexToken* lToken)
{
    GUIN_increment_lexer(lState, 1);

    #define GUIN_rewind_string_lexer() {\
        GUIN_clearstring(&lToken->string);\
        lToken->string = GUIN_init_String();\
        lToken->length = 1;\
        GUIN_increment_lexer(lState, -((lState->current_column)-original));\
    }

    lToken->length = 1;
    size_t original = lState->current_column;
    for (;true;GUIN_increment_lexer(lState, 1)) {
        ++lToken->length;
        // reached EOF
        if (lState->index >= lState->str_length) {
            if (lToken->type == GUIN_TK_ERR) {
                break;
            }
                    
            lState->errmsg = "String was left unterminated and reached <EOF>!";
            lToken->type = GUIN_TK_ERR;
            GUIN_rewind_string_lexer();
            GUIN_stringaddchar(&lToken->string, '"');
        }
        // reached end of line
        if (GUIN_lState_char == '\n') {
            if (lToken->type == GUIN_TK_ERR) {
                break;
            }

            lState->errmsg = "String was left unterminated!";
            lToken->type = GUIN_TK_ERR;
            GUIN_rewind_string_lexer();
            GUIN_stringaddchar(&lToken->string, '"');
        }

        // Reached the end of string
        if (GUIN_lState_char == '"') {
            GUIN_increment_lexer(lState, 1);
            if (lToken->type == GUIN_TK_ERR) {
                ++lToken->length;
                GUIN_stringaddchar(&lToken->string, '"');
            }
            break;
        }

        if (GUIN_lState_char == '\\' && lToken->type != GUIN_TK_ERR) {
            GUIN_increment_lexer(lState, 1);
            struct GUIN_ParseEscapeCode ret = GUIN_parse_escape_code(lState->string, lState->index);

            lToken->length += ret.size;
            if (ret.err) {
                lState->errmsg = "Invalid escape character!";
                lToken->type = GUIN_TK_ERR;
                GUIN_rewind_string_lexer();
                GUIN_stringaddchar(&lToken->string, '"');
                GUIN_stringaddchar(&lToken->string, GUIN_lState_char);
            } else {
                GUIN_increment_lexer(lState, ret.size-1);
                GUIN_stringaddchar(&lToken->string, ret.ch);
            }
        } else {
            //++lToken->length;
            GUIN_stringaddchar(&lToken->string, GUIN_lState_char);
        }
    }

    if (lToken->type != GUIN_TK_ERR)
        lToken->type   = GUIN_TK_String_val;
}




GUIN_LexState GUIN_init_LexState(GUIN_String* file_content)
{
    GUIN_LexState x;

    x.current_column = 1;
    x.current_line   = 1;

    x.string         = file_content->content;
    x.str_length     = file_content->length;
    x.index          = 0;

    x.errmsg         = NULL;

    return x;
}





GUIN_LexToken GUIN_advance_lexer(GUIN_LexState* lState)
{
    GUIN_LexToken lToken = {0};
    lToken.column = lState->current_column;
    lToken.line   = lState->current_line;
    lToken.length = 1;

    if (lState->index >= lState->str_length) {
        lToken.type = GUIN_TK_EOF;
        return lToken;
    }

    bool doing_comment       = false;
    bool doing_multi_comment = false;

    for (; lState->index < lState->str_length; GUIN_increment_lexer(lState, 1)) {
        if (GUIN_lState_char == ' ' || GUIN_lState_char == '\t') {
            continue;
        }
        if (GUIN_lState_char == '\n') {
            lState->current_column = 0;
            ++lState->current_line;
            doing_comment = false;
            continue;
        }
        if (GUIN_lState_char == '\v') {
            continue;
        }
        if (GUIN_lState_char == '\r') {
            lState->current_column = 0;
            continue;
        }
        if (doing_comment) {
            continue;
        }
        if (GUIN_lState_char == '/' && GUIN_ahead_lState_char == '*') {
            doing_multi_comment = true;
            continue;    
        }
        if (GUIN_lState_char == '*' && GUIN_ahead_lState_char == '/') {
            if (!doing_multi_comment) {
                lState->errmsg = "End of a multi-lined comment mentioned when it wasn't active!";
                lToken.column = lState->current_column;
                lToken.line   = lState->current_line;
                lToken.string = GUIN_init_String();
                GUIN_stringconcat_charptr(&lToken.string, "*/");
                lToken.type = GUIN_TK_ERR;
                return lToken;
            }

            doing_multi_comment = false;

            // so it effectively skips 2 chars
            ++lState->index;
            continue;

        } else if (GUIN_lState_char == '/' && GUIN_ahead_lState_char == '/' && !doing_multi_comment) {
            doing_comment = true;
            continue;
        }
        if (doing_multi_comment) {
            continue;
        }

        lToken.column = lState->current_column;
        lToken.line   = lState->current_line;

        // CHECK IF ITS A 2 CHAR SIZED THING LIKE ==, !=, etc...
        for (size_t i = 0; i < sizeof(doublechartoken)/sizeof(struct GUIN_DoubleOperationChar); i++) {
            if (doublechartoken[i].f_ch == GUIN_lState_char && doublechartoken[i].s_ch == GUIN_ahead_lState_char) {
                lToken.type = doublechartoken[i].type;
                GUIN_increment_lexer(lState, 2);
                return lToken;
            }
        }

        // CHECK IF ITS A 1 CHAR SIZED THING LIKE =, <, etc...
        for (size_t i = 0; i < sizeof(chartoken)/sizeof(struct GUIN_LexNodeCharToEnumData); i++) {
            if (chartoken[i].ch == GUIN_lState_char) {
                lToken.type = chartoken[i].type;
                GUIN_increment_lexer(lState, 1);
                return lToken;
            }
        }

        lToken.string = GUIN_init_String();

        // Doing string
        if (GUIN_lState_char == '"') {
            GUIN_parse_lexer_string(lState, &lToken);
            return lToken;
        }
        
        lToken.length = 0;

        // Checking if its just invalid characters and if it is then collect
        if (!GUIN_is_valid_identifier_char(GUIN_lState_char)) {
            for (;!GUIN_is_valid_identifier_char(GUIN_lState_char) && GUIN_lState_char != ' ' && GUIN_lState_char != '\n' && GUIN_lState_char != '\t'; GUIN_increment_lexer(lState, 1)) {
                GUIN_stringaddchar(&lToken.string, GUIN_lState_char);
            }
            lToken.type = GUIN_TK_UNKNOWN;
        } else {
            char doing_decimal = 0;

            // If its invalid then collect the identifier/number
            for (;true;GUIN_increment_lexer(lState, 1)) {
                if (GUIN_is_valid_identifier_char(GUIN_lState_char)) {
                    GUIN_stringaddchar(&lToken.string, GUIN_lState_char);
                    ++lToken.length;
                } else {
                    // if its a number accept the . to be a float
                    if (!doing_decimal && GUIN_lState_char == '.' && GUIN_is_int_string(lToken.string.content)) {
                        GUIN_stringaddchar(&lToken.string, '.');
                        doing_decimal = 1;
                        ++lToken.length;
                    } else if ((doing_decimal < 2 && doing_decimal != 0) && (GUIN_is_valid_identifier_char(GUIN_lState_char) || GUIN_lState_char == '.')) {
                        GUIN_stringaddchar(&lToken.string, GUIN_lState_char);
                        ++doing_decimal;
                        ++lToken.length;
                    } else {
                        break;
                    }
                }
            }
            GUIN_set_token_type(&lToken, lState);
        }

        if (lToken.type == GUIN_TK_UNKNOWN && lState->errmsg == NULL)
            lState->errmsg = "Unknown token!";

        return lToken;
    }

    // If we reached this far, it means 
    // we went through all whitespace 
    // and reached the end
    lToken.column = lState->current_column;
    lToken.line   = lState->current_line;
    lToken.type = GUIN_TK_EOF;
    return lToken;
}


void GUIN_destroy_LexToken(GUIN_LexToken* lToken)
{
    if (lToken->type == GUIN_TK_String_val || lToken->type == GUIN_TK_Identifier || lToken->type == GUIN_TK_UNKNOWN || lToken->type == GUIN_TK_ERR)
        GUIN_clearstring(&lToken->string);

    lToken->type   = GUIN_TK_UNINIT;
    lToken->line   = 0;
    lToken->column = 0;
}


#endif
