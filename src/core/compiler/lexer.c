#ifndef LEXER_C
#define LEXER_C

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "lexer.h"
#include "../../etc/string-manipulation.h"

char* guin_LexTokenEnum_to_string(guin_LexTokenEnum e)
{
    switch (e)
    {
        case guin_TK_Identifier:     return "identifier";
        case guin_TK_var:            return "var";

        case guin_TK_Int_val:        return "(int)";
        case guin_TK_Number_val:     return "(number)";
        case guin_TK_String_val:     return "(string)";
        case guin_TK_Char_val:       return "(char)";
        case guin_TK_Bool_val:       return "(bool)";
        case guin_TK_nil:            return "nil";
        case guin_TK_NaN:            return "NaN";

        case guin_TK_if:             return "if";
        case guin_TK_else:           return "else";
        case guin_TK_while:          return "while";
        case guin_TK_for:            return "for";
        case guin_TK_return:         return "return";
        case guin_TK_continue:       return "continue";
        case guin_TK_break:          return "break";

        case guin_TK_func:           return "func";
        case guin_TK_int:            return "int";
        case guin_TK_number:         return "number";
        case guin_TK_string:         return "string";
        case guin_TK_char:           return "char";
        case guin_TK_bool:           return "bool";
        case guin_TK_void:           return "void";

        case guin_TK_ASSIGN:         return "=";
        case guin_TK_ADD:            return "+";
        case guin_TK_SUB:            return "-";
        case guin_TK_MUL:            return "*";
        case guin_TK_DIV:            return "/";
        case guin_TK_MOD:            return "%";
        case guin_TK_POW:            return "^";

        case guin_TK_HASH:           return "#";

        case guin_TK_PARENTHESIS_L:  return "(";
        case guin_TK_PARENTHESIS_R:  return ")";
        case guin_TK_COLON:          return ":";
        case guin_TK_SEMI_COLON:     return ";";
        case guin_TK_DOT:            return ".";
        case guin_TK_COMMA:          return ",";

        case guin_TK_GT:             return ">";
        case guin_TK_LT:             return "<";
        case guin_TK_EQU:            return "==";
        case guin_TK_NOT_EQU:        return "!=";
        case guin_TK_LT_EQU:         return "<=";
        case guin_TK_GT_EQU:         return ">=";
        case guin_TK_and:            return "and";
        case guin_TK_or:             return "or";

        case guin_TK_CURLY_L:        return "{";
        case guin_TK_CURLY_R:        return "}";

        case guin_TK_BRACKET_L:      return "[";
        case guin_TK_BRACKET_R:      return "]";

        case guin_TK_EOF:            return "EOF";
        case guin_TK_UNINIT:         return "UNINIT";
        case guin_TK_UNKNOWN:        return "UNKNOWN";
        case guin_TK_ERR:            return "ERR";

        default:                return "(UNAVAILABLE)";
    }
}

struct guin_LexNodeStringToEnumData {
    char* string;
    guin_LexTokenEnum type;
};

struct guin_LexNodeCharToEnumData {
    char ch;
    guin_LexTokenEnum type;
};

static const struct guin_LexNodeStringToEnumData stringtoken[] = {
    {.string="true",        .type=guin_TK_Bool_val},
    {.string="false",       .type=guin_TK_Bool_val},
    {.string="nil",         .type=guin_TK_nil},
    {.string="void",        .type=guin_TK_void},
    {.string="NaN",         .type=guin_TK_NaN},
    {.string="Inf",         .type=guin_TK_Inf},

    {.string="func",        .type=guin_TK_func},
    {.string="int",         .type=guin_TK_int},
    {.string="number",      .type=guin_TK_number},
    {.string="char",        .type=guin_TK_char},
    {.string="bool",        .type=guin_TK_bool},
    {.string="string",      .type=guin_TK_string},
    {.string="dynamic",     .type=guin_TK_dynamic},

    {.string="if",          .type=guin_TK_if},
    {.string="else",        .type=guin_TK_else},
    {.string="while",       .type=guin_TK_while},
    {.string="for",         .type=guin_TK_for},
    {.string="not",         .type=guin_TK_not},
    {.string="and",         .type=guin_TK_and},
    {.string="or",          .type=guin_TK_or},
    {.string="continue",    .type=guin_TK_continue},
    {.string="break",       .type=guin_TK_break},
    {.string="return",      .type=guin_TK_return},

    {.string="var",         .type=guin_TK_var},
};

static const struct guin_LexNodeCharToEnumData chartoken[] = {
    {.ch='{',       .type=guin_TK_CURLY_L},
    {.ch='}',       .type=guin_TK_CURLY_R},
    {.ch='[',       .type=guin_TK_BRACKET_L},
    {.ch=']',       .type=guin_TK_BRACKET_R},

    {.ch='(',       .type=guin_TK_PARENTHESIS_L},
    {.ch=')',       .type=guin_TK_PARENTHESIS_R},
    {.ch='.',       .type=guin_TK_DOT},
    {.ch=':',       .type=guin_TK_COLON},
    {.ch=';',       .type=guin_TK_SEMI_COLON},
    {.ch=',',       .type=guin_TK_COMMA},

    {.ch='=',       .type=guin_TK_ASSIGN},
    {.ch='<',       .type=guin_TK_LT},
    {.ch='>',       .type=guin_TK_GT},
    {.ch='+',       .type=guin_TK_ADD},
    {.ch='-',       .type=guin_TK_SUB},
    {.ch='*',       .type=guin_TK_MUL},
    {.ch='/',       .type=guin_TK_DIV},
    {.ch='%',       .type=guin_TK_MOD},
    {.ch='^',       .type=guin_TK_POW},
    {.ch='#',       .type=guin_TK_HASH},
};

struct guin_DoubleOperationChar {
    char f_ch;
    char s_ch;
    guin_LexTokenEnum type;
};

static const struct guin_DoubleOperationChar doublechartoken[] = {
    {.f_ch='=', .s_ch='=',      .type=guin_TK_EQU},
    {.f_ch='!', .s_ch='=',      .type=guin_TK_NOT_EQU},
    {.f_ch='<', .s_ch='=',      .type=guin_TK_LT_EQU},
    {.f_ch='>', .s_ch='=',      .type=guin_TK_GT_EQU},
};

static void guin_set_token_type(guin_LexToken* token, guin_LexState* lState) // for true/false also converts into right datatype
{
    if (guin_is_int_string(token->string.content) || guin_is_valid_number(token->string.content)) {
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

            token->type = (found) ? guin_TK_Number_val : guin_TK_Int_val;
        }

        guin_int64     integer = 0;
        guin_number64  number  = 0;
        guin_number64  decimal = 0;

        if (token->type == guin_TK_Int_val) {
            for (size_t i = 0; i < token->string.length; i++) {
                integer = (integer * 10) + guin_to_number(token->string.content[i]);
            }
        } else {
            size_t i = 0;
            for (; i < decimal_index; i++) {
                number = (number * 10) + guin_to_number(token->string.content[i]);
            }
            size_t current_dec_pos = 1;
            for (i += 1; i < token->string.length; i++) {
                decimal = guin_to_number(token->string.content[i]);
                if (decimal != 0) decimal /= pow(10, current_dec_pos);
                number += decimal;
                ++current_dec_pos;
            }
        }

        guin_clearstring(&token->string);

        if (token->type == guin_TK_Number_val) {
            token->number  = number;
        } else { 
            token->integer = integer; 
        }

        return;
    }

    for (size_t i = 0; i < sizeof(stringtoken)/sizeof(struct guin_LexNodeStringToEnumData); i++) {
        if (strcmp(stringtoken[i].string, token->string.content) == 0) {
            guin_clearstring(&token->string);
            token->type = stringtoken[i].type;

            if (token->type == guin_TK_Bool_val)
                token->bl = stringtoken[i].string[0] == 't'; // if true, then 1 else 0

            else if (token->type == guin_TK_NaN) {
                token->type = guin_TK_Number_val;
                token->number = guin_NaN;
            }

            else if (token->type == guin_TK_Inf) {
                token->type = guin_TK_Number_val;
                token->number = guin_Inf;
            }

            return;
        }
    }

    if (guin_is_valid_identifier_string(token->string.content)) {
        token->type = guin_TK_Identifier;
        return;
    }

    token->type = guin_TK_UNKNOWN;
    return;
}

struct guin_EscapeCodeToInt {
    char ch;
    char convert;
};

static const struct guin_EscapeCodeToInt escapeCodeToNumber[] = {
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

struct guin_ParseEscapeCode {
    char size;
    char ch;
    bool err;
};

static struct guin_ParseEscapeCode guin_parse_escape_code(char* str, size_t index)
{
    struct guin_ParseEscapeCode ret = {.ch=0,.err=false,.size=0};
    size_t len = 0;
    while (str[index+len] != 0) ++len;

    // IF ITS LIKE \n OR \t
    for (size_t i = 0; i < sizeof(escapeCodeToNumber)/sizeof(struct guin_EscapeCodeToInt); i++) {
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
    if (guin_is_octal_char(str[index])) {
        for (; guin_is_octal_char(str[index+ret.size]) && ret.size < 3; ret.size++)
            ret.ch = (ret.ch * 8) + guin_to_number(str[index+ret.size]);

        if (ret.size > 0) return ret;
    }

    // for '\x' AND '\o'
    
    ++ret.size;
    switch (str[index]) {
        case 'x':
            for (; guin_is_hex_code(str[index+ret.size]) && ret.size < 3; ret.size++)
                ret.ch = (ret.ch * 16) + guin_to_hex_char(str[index+ret.size]);
            if (ret.size > 1) return ret;

        case 'o':
            for (; guin_is_octal_char(str[index+ret.size]) && ret.size < 4; ret.size++)
                ret.ch = (ret.ch * 8) + guin_to_number(str[index+ret.size]);
            if (ret.size > 1) return ret;
    }

    ret.size = 0;
    ret.err = true;
    return ret;
}

#define guin_lState_char       (lState->string[lState->index])
#define guin_ahead_lState_char (lState->string[lState->index + 1])

static void guin_increment_lexer(guin_LexState* lState, size_t incremental_size)
{
    lState->current_column += incremental_size;
    lState->index          += incremental_size;
}

static void guin_parse_lexer_string(guin_LexState* lState, guin_LexToken* lToken)
{
    guin_increment_lexer(lState, 1);

    #define guin_rewind_string_lexer() {\
        guin_clearstring(&lToken->string);\
        lToken->string = guin_init_String();\
        lToken->length = 1;\
        guin_increment_lexer(lState, -((lState->current_column)-original));\
    }

    lToken->length = 1;
    size_t original = lState->current_column;
    for (;true;guin_increment_lexer(lState, 1)) {
        ++lToken->length;
        // reached EOF
        if (lState->index >= lState->str_length) {
            if (lToken->type == guin_TK_ERR) {
                break;
            }
                    
            lState->errmsg = "String was left unterminated and reached <EOF>!";
            lToken->type = guin_TK_ERR;
            guin_rewind_string_lexer();
            guin_stringaddchar(&lToken->string, '"');
        }
        // reached end of line
        if (guin_lState_char == '\n') {
            if (lToken->type == guin_TK_ERR) {
                break;
            }

            lState->errmsg = "String was left unterminated!";
            lToken->type = guin_TK_ERR;
            guin_rewind_string_lexer();
            guin_stringaddchar(&lToken->string, '"');
        }

        // Reached the end of string
        if (guin_lState_char == '"') {
            guin_increment_lexer(lState, 1);
            if (lToken->type == guin_TK_ERR) {
                ++lToken->length;
                guin_stringaddchar(&lToken->string, '"');
            }
            break;
        }

        if (guin_lState_char == '\\' && lToken->type != guin_TK_ERR) {
            guin_increment_lexer(lState, 1);
            struct guin_ParseEscapeCode ret = guin_parse_escape_code(lState->string, lState->index);

            lToken->length += ret.size;
            if (ret.err) {
                lState->errmsg = "Invalid escape character!";
                lToken->type = guin_TK_ERR;
                guin_rewind_string_lexer();
                guin_stringaddchar(&lToken->string, '"');
                guin_stringaddchar(&lToken->string, guin_lState_char);
            } else {
                guin_increment_lexer(lState, ret.size-1);
                guin_stringaddchar(&lToken->string, ret.ch);
            }
        } else {
            //++lToken->length;
            guin_stringaddchar(&lToken->string, guin_lState_char);
        }
    }

    if (lToken->type != guin_TK_ERR)
        lToken->type   = guin_TK_String_val;
}




guin_LexState guin_init_LexState(guin_String* file_content)
{
    guin_LexState x;

    x.current_column = 1;
    x.current_line   = 1;

    x.string         = file_content->content;
    x.str_length     = file_content->length;
    x.index          = 0;

    x.errmsg         = NULL;

    return x;
}





guin_LexToken guin_advance_lexer(guin_LexState* lState)
{
    guin_LexToken lToken = {0};
    lToken.column = lState->current_column;
    lToken.line   = lState->current_line;
    lToken.length = 1;

    if (lState->index >= lState->str_length) {
        lToken.type = guin_TK_EOF;
        return lToken;
    }

    bool doing_comment       = false;
    bool doing_multi_comment = false;

    for (; lState->index < lState->str_length; guin_increment_lexer(lState, 1)) {
        if (guin_lState_char == ' ' || guin_lState_char == '\t') {
            continue;
        }
        if (guin_lState_char == '\n') {
            lState->current_column = 0;
            ++lState->current_line;
            doing_comment = false;
            continue;
        }
        if (guin_lState_char == '\v') {
            continue;
        }
        if (guin_lState_char == '\r') {
            lState->current_column = 0;
            continue;
        }
        if (doing_comment) {
            continue;
        }
        if (guin_lState_char == '/' && guin_ahead_lState_char == '*') {
            doing_multi_comment = true;
            continue;    
        }
        if (guin_lState_char == '*' && guin_ahead_lState_char == '/') {
            if (!doing_multi_comment) {
                lState->errmsg = "End of a multi-lined comment mentioned when it wasn't active!";
                lToken.column = lState->current_column;
                lToken.line   = lState->current_line;
                lToken.string = guin_init_String();
                guin_stringconcat_charptr(&lToken.string, "*/");
                lToken.type = guin_TK_ERR;
                return lToken;
            }

            doing_multi_comment = false;

            // so it effectively skips 2 chars
            ++lState->index;
            continue;

        } else if (guin_lState_char == '/' && guin_ahead_lState_char == '/' && !doing_multi_comment) {
            doing_comment = true;
            continue;
        }
        if (doing_multi_comment) {
            continue;
        }

        lToken.column = lState->current_column;
        lToken.line   = lState->current_line;

        // CHECK IF ITS A 2 CHAR SIZED THING LIKE ==, !=, etc...
        for (size_t i = 0; i < sizeof(doublechartoken)/sizeof(struct guin_DoubleOperationChar); i++) {
            if (doublechartoken[i].f_ch == guin_lState_char && doublechartoken[i].s_ch == guin_ahead_lState_char) {
                lToken.type = doublechartoken[i].type;
                guin_increment_lexer(lState, 2);
                return lToken;
            }
        }

        // CHECK IF ITS A 1 CHAR SIZED THING LIKE =, <, etc...
        for (size_t i = 0; i < sizeof(chartoken)/sizeof(struct guin_LexNodeCharToEnumData); i++) {
            if (chartoken[i].ch == guin_lState_char) {
                lToken.type = chartoken[i].type;
                guin_increment_lexer(lState, 1);
                return lToken;
            }
        }

        lToken.string = guin_init_String();

        // Doing string
        if (guin_lState_char == '"') {
            guin_parse_lexer_string(lState, &lToken);
            return lToken;
        }
        
        lToken.length = 0;

        // Checking if its just invalid characters and if it is then collect
        if (!guin_is_valid_identifier_char(guin_lState_char)) {
            for (;!guin_is_valid_identifier_char(guin_lState_char) && guin_lState_char != ' ' && guin_lState_char != '\n' && guin_lState_char != '\t'; guin_increment_lexer(lState, 1)) {
                guin_stringaddchar(&lToken.string, guin_lState_char);
            }
            lToken.type = guin_TK_UNKNOWN;
        } else {
            char doing_decimal = 0;

            // If its invalid then collect the identifier/number
            for (;true;guin_increment_lexer(lState, 1)) {
                if (guin_is_valid_identifier_char(guin_lState_char)) {
                    guin_stringaddchar(&lToken.string, guin_lState_char);
                    ++lToken.length;
                } else {
                    // if its a number accept the . to be a float
                    if (!doing_decimal && guin_lState_char == '.' && guin_is_int_string(lToken.string.content)) {
                        guin_stringaddchar(&lToken.string, '.');
                        doing_decimal = 1;
                        ++lToken.length;
                    } else if ((doing_decimal < 2 && doing_decimal != 0) && (guin_is_valid_identifier_char(guin_lState_char) || guin_lState_char == '.')) {
                        guin_stringaddchar(&lToken.string, guin_lState_char);
                        ++doing_decimal;
                        ++lToken.length;
                    } else {
                        break;
                    }
                }
            }
            guin_set_token_type(&lToken, lState);
        }

        if (lToken.type == guin_TK_UNKNOWN && lState->errmsg == NULL)
            lState->errmsg = "Unknown token!";

        return lToken;
    }

    // If we reached this far, it means 
    // we went through all whitespace 
    // and reached the end
    lToken.column = lState->current_column;
    lToken.line   = lState->current_line;
    lToken.type = guin_TK_EOF;
    return lToken;
}


void guin_destroy_LexToken(guin_LexToken* lToken)
{
    if (lToken->type == guin_TK_String_val || lToken->type == guin_TK_Identifier || lToken->type == guin_TK_UNKNOWN || lToken->type == guin_TK_ERR)
        guin_clearstring(&lToken->string);

    lToken->type   = guin_TK_UNINIT;
    lToken->line   = 0;
    lToken->column = 0;
}


#endif
