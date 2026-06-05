#ifndef LEXER_C
#define LEXER_C

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "lexer.h"
#include "../../etc/string_manipulation.h"

char* LexTokenEnum_to_string(LexTokenEnum e)
{
    switch (e)
    {
        case TK_Identifier:     return "identifier";
        case TK_var:            return "var";

        case TK_Int_val:        return "(int)";
        case TK_Number_val:     return "(number)";
        case TK_String_val:     return "(string)";
        case TK_Char_val:       return "(char)";
        case TK_Bool_val:       return "(bool)";
        case TK_nil:            return "nil";

        case TK_if:             return "if";
        case TK_then:           return "then";
        case TK_else:           return "else";
        case TK_while:          return "while";
        case TK_for:            return "for";
        case TK_do:             return "do";
        case TK_end:            return "end";
        case TK_return:         return "return";
        case TK_continue:       return "continue";
        case TK_break:          return "break";

        case TK_function:       return "function";
        case TK_int:            return "int";
        case TK_number:         return "number";
        case TK_string:         return "string";
        case TK_char:           return "char";
        case TK_bool:           return "bool";

        case TK_ASSIGN:         return "=";
        case TK_ADD:            return "+";
        case TK_SUB:            return "-";
        case TK_MUL:            return "*";
        case TK_DIV:            return "/";
        case TK_MOD:            return "%";
        case TK_POW:            return "^";

        case TK_HASH:           return "#";

        case TK_PARENTHESIS_L:  return "(";
        case TK_PARENTHESIS_R:  return ")";
        case TK_COLON:          return ":";
        case TK_SEMI_COLON:     return ";";
        case TK_DOT:            return ".";
        case TK_COMMA:          return ",";

        case TK_GT:             return ">";
        case TK_LT:             return "<";
        case TK_EQU:            return "==";
        case TK_NOT_EQU:        return "!=";
        case TK_LT_EQU:         return "<=";
        case TK_GT_EQU:         return ">=";
        case TK_and:            return "and";
        case TK_or:             return "or";

        case TK_CURLY_L:        return "{";
        case TK_CURLY_R:        return "}";

        case TK_BRACKET_L:      return "[";
        case TK_BRACKET_R:      return "]";

        case TK_EOF:            return "EOF";
        case TK_UNINIT:         return "UNINIT";
        case TK_UNKNOWN:        return "UNKNOWN";
        case TK_ERR:            return "ERR";

        default:                return "(UNAVAILABLE)";
    }
}

struct LexNodeStringToEnumData {
    char* string;
    LexTokenEnum type;
};

struct LexNodeCharToEnumData {
    char ch;
    LexTokenEnum type;
};

const struct LexNodeStringToEnumData stringtoken[] = {
    {.string="true",        .type=TK_Bool_val},
    {.string="false",       .type=TK_Bool_val},
    {.string="nil",         .type=TK_nil},

    {.string="function",    .type=TK_function},
    {.string="int",         .type=TK_int},
    {.string="number",      .type=TK_number},
    {.string="char",        .type=TK_char},
    {.string="bool",        .type=TK_bool},
    {.string="string",      .type=TK_string},

    {.string="if",          .type=TK_if},
    {.string="else",        .type=TK_else},
    {.string="while",       .type=TK_while},
    {.string="for",         .type=TK_for},
    {.string="end",         .type=TK_end},
    {.string="do",          .type=TK_do},
    {.string="then",        .type=TK_then},
    {.string="and",         .type=TK_and},
    {.string="or",          .type=TK_or},

    {.string="var",         .type=TK_var},
};

const struct LexNodeCharToEnumData chartoken[] = {
    {.ch='{',       .type=TK_CURLY_L},
    {.ch='}',       .type=TK_CURLY_R},
    {.ch='[',       .type=TK_BRACKET_L},
    {.ch=']',       .type=TK_BRACKET_R},

    {.ch='(',       .type=TK_PARENTHESIS_L},
    {.ch=')',       .type=TK_PARENTHESIS_R},
    {.ch='.',       .type=TK_DOT},
    {.ch=':',       .type=TK_COLON},
    {.ch=';',       .type=TK_SEMI_COLON},
    {.ch=',',       .type=TK_COMMA},

    {.ch='=',       .type=TK_ASSIGN},
    {.ch='<',       .type=TK_LT},
    {.ch='>',       .type=TK_GT},
    {.ch='+',       .type=TK_ADD},
    {.ch='-',       .type=TK_SUB},
    {.ch='*',       .type=TK_MUL},
    {.ch='/',       .type=TK_DIV},
    {.ch='%',       .type=TK_MOD},
    {.ch='^',       .type=TK_POW},
    {.ch='#',       .type=TK_HASH},
};

struct DoubleOperationChar {
    char f_ch;
    char s_ch;
    LexTokenEnum type;
};

const struct DoubleOperationChar doublechartoken[] = {
    {.f_ch='=', .s_ch='=',      .type=TK_EQU},
    {.f_ch='!', .s_ch='=',      .type=TK_NOT_EQU},
    {.f_ch='<', .s_ch='=',      .type=TK_LT_EQU},
    {.f_ch='>', .s_ch='=',      .type=TK_GT_EQU},
};

void set_token_type(LexToken* token, LexState* lState) // for true/false also converts into right datatype
{
    if (is_int_string(token->string.content) || is_valid_number(token->string.content)) {
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

            token->type = (found) ? TK_Number_val : TK_Int_val;
        }

        int64_t integer = 0;
        double  number  = 0;
        double  decimal = 0;

        if (token->type == TK_Int_val) {
            for (size_t i = 0; i < token->string.length; i++) {
                integer = (integer * 10) + to_number(token->string.content[i]);
            }
        } else {
            size_t i = 0;
            for (; i < decimal_index; i++) {
                number = (number * 10) + to_number(token->string.content[i]);
            }
            size_t current_dec_pos = 1;
            for (i += 1; i < token->string.length; i++) {
                decimal = to_number(token->string.content[i]);
                if (decimal != 0) decimal /= pow(10, current_dec_pos);
                number += decimal;
                ++current_dec_pos;
            }
        }

        clearstring(&token->string);

        if (token->type == TK_Number_val) {
            token->number  = number;
        } else { 
            token->integer = integer; 
        }

        return;
    }

    for (size_t i = 0; i < sizeof(stringtoken)/sizeof(struct LexNodeStringToEnumData); i++) {
        if (strcmp(stringtoken[i].string, token->string.content) == 0) {
            clearstring(&token->string);
            token->type = stringtoken[i].type;

            if (token->type == TK_Bool_val)
                token->bl = stringtoken[i].string[0] == 't'; // if true, then 1 else 0

            return;
        }
    }

    if (is_valid_identifier_string(token->string.content)) {
        token->type = TK_Identifier;
        return;
    }

    token->type = TK_UNKNOWN;
    return;
}

struct EscapeCodeToInt {
    char ch;
    char convert;
};

const struct EscapeCodeToInt escapeCodeToNumber[] = {
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

struct ParseEscapeCode {
    char size;
    char ch;
    bool err;
};

struct ParseEscapeCode parse_escape_code(char* str, size_t index)
{
    struct ParseEscapeCode ret = {.ch=0,.err=false,.size=0};
    size_t len = 0;
    while (str[index+len] != 0) ++len;

    // IF ITS LIKE \n OR \t
    for (size_t i = 0; i < sizeof(escapeCodeToNumber)/sizeof(struct EscapeCodeToInt); i++) {
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
    if (is_octal_char(str[index])) {
        for (; is_octal_char(str[index+ret.size]) && ret.size < 3; ret.size++)
            ret.ch = (ret.ch * 8) + to_number(str[index+ret.size]);

        if (ret.size > 0) return ret;
    }

    // for '\x' AND '\o'
    
    ++ret.size;
    switch (str[index]) {
        case 'x':
            for (; is_hex_code(str[index+ret.size]) && ret.size < 3; ret.size++)
                ret.ch = (ret.ch * 16) + to_hex_char(str[index+ret.size]);
            if (ret.size > 1) return ret;

        case 'o':
            for (; is_octal_char(str[index+ret.size]) && ret.size < 4; ret.size++)
                ret.ch = (ret.ch * 8) + to_number(str[index+ret.size]);
            if (ret.size > 1) return ret;
    }

    ret.size = 0;
    ret.err = true;
    return ret;
}

#define lState_char       (lState->string[lState->index])
#define ahead_lState_char (lState->string[lState->index + 1])

void increment_lexer(LexState* lState, size_t incremental_size)
{
    lState->current_column += incremental_size;
    lState->index          += incremental_size;
}

void parse_lexer_string(LexState* lState, LexToken* lToken)
{
    increment_lexer(lState, 1);

    #define rewind_string_lexer() {\
        clearstring(&lToken->string);\
        lToken->string = init_String();\
        increment_lexer(lState, -((lState->current_column)-original));\
    }

    size_t original = lState->current_column;
    for (;true;increment_lexer(lState, 1)) {
        // reached EOF
        if (lState->index >= lState->str_length) {
            if (lToken->type == TK_ERR) {
                break;
            }
                    
            lState->errmsg = "String was left unterminated and reached <EOF>!";
            lToken->type = TK_ERR;
            rewind_string_lexer();
            stringaddchar(&lToken->string, '"');
        }
        // reached end of line
        if (lState_char == '\n') {
            if (lToken->type == TK_ERR) {
                break;
            }

            lState->errmsg = "String was left unterminated!";
            lToken->type = TK_ERR;
            rewind_string_lexer();
            stringaddchar(&lToken->string, '"');
        }

        // Reached the end of string
        if (lState_char == '"') {
            increment_lexer(lState, 1);
            if (lToken->type == TK_ERR)
                stringaddchar(&lToken->string, '"');
            break;
        }

        if (lState_char == '\\' && lToken->type != TK_ERR) {
            increment_lexer(lState, 1);
            struct ParseEscapeCode ret = parse_escape_code(lState->string, lState->index);

            if (ret.err) {
                lState->errmsg = "Invalid escape character!";
                lToken->type = TK_ERR;
                rewind_string_lexer();
                stringaddchar(&lToken->string, '"');
                stringaddchar(&lToken->string, lState_char);
            } else {
                increment_lexer(lState, ret.size-1);
                stringaddchar(&lToken->string, ret.ch);
            }
        } else {
            stringaddchar(&lToken->string, lState_char);
        }
    }

    if (lToken->type != TK_ERR)
        lToken->type = TK_String_val;
}




LexState init_LexState(String* file_content)
{
    LexState x;

    x.current_column = 1;
    x.current_line   = 1;

    x.string         = file_content->content;
    x.str_length     = file_content->length;
    x.index          = 0;

    x.errmsg         = NULL;

    return x;
}





LexToken advance_lexer(LexState* lState)
{
    LexToken lToken = {0};
    lToken.column = lState->current_column;
    lToken.line   = lState->current_line;

    if (lState->index >= lState->str_length) {
        lToken.type = TK_EOF;
        return lToken;
    }

    bool doing_comment = false;

    for (; lState->index < lState->str_length; increment_lexer(lState, 1)) {
        if (lState_char == ' ') {
            continue;
        }
        if (lState_char == '\n') {
            lState->current_column = 0;
            ++lState->current_line;
            doing_comment = false;
            continue;
        }
        if (lState_char == '\v') {
            continue;
        }
        if (lState_char == '\r') {
            lState->current_column = 0;
            continue;
        }
        if (doing_comment) {
            continue;
        }
        if (lState_char == '/' && ahead_lState_char == '/') {
            doing_comment = true;
            continue;
        }

        lToken.column = lState->current_column;
        lToken.line   = lState->current_line;

        // CHECK IF ITS A 2 CHAR SIZED THING LIKE ==, !=, etc...
        for (size_t i = 0; i < sizeof(doublechartoken)/sizeof(struct DoubleOperationChar); i++) {
            if (doublechartoken[i].f_ch == lState_char && doublechartoken[i].s_ch == ahead_lState_char) {
                lToken.type = doublechartoken[i].type;
                increment_lexer(lState, 2);
                return lToken;
            }
        }

        // CHECK IF ITS A 1 CHAR SIZED THING LIKE =, <, etc...
        for (size_t i = 0; i < sizeof(chartoken)/sizeof(struct LexNodeCharToEnumData); i++) {
            if (chartoken[i].ch == lState_char) {
                lToken.type = chartoken[i].type;
                increment_lexer(lState, 1);
                return lToken;
            }
        }

        bool done_decimal = false;
        bool error        = false;
        lToken.string = init_String();

        // Doing string
        if (lState_char == '"') {
            parse_lexer_string(lState, &lToken);
            return lToken;
        }
        
        // Checking if its just invalid characters and if it is then collect
        if (!is_valid_identifier_char(lState_char)) {
            for (;!is_valid_identifier_char(lState_char) && lState_char != ' ' && lState_char != '\n' && lState_char != '\t'; increment_lexer(lState, 1)) {
                stringaddchar(&lToken.string, lState_char);
            }
            lToken.type = TK_UNKNOWN;
        } else {
            double decimal = 0;
            char doing_decimal = 0;

            // If its invalid then collect the identifier/number
            for (;true;increment_lexer(lState, 1)) {
                if (is_valid_identifier_char(lState_char)) {
                    stringaddchar(&lToken.string, lState_char);
                } else {
                    // if its a number accept the . to be a float
                    if (!doing_decimal && lState_char == '.' && is_int_string(lToken.string.content)) {
                        stringaddchar(&lToken.string, '.');
                        doing_decimal = 1;
                    } else if ((doing_decimal < 2 && doing_decimal != 0) && (is_valid_identifier_char(lState_char) || lState_char == '.')) {
                        stringaddchar(&lToken.string, lState_char);
                        ++doing_decimal;
                    } else {
                        break;
                    }
                }
            }
            set_token_type(&lToken, lState);
        }

        if (lToken.type == TK_UNKNOWN && lState->errmsg == NULL)
            lState->errmsg = "Unknown token!";

        return lToken;
    }

    // If we reached this far, it means 
    // we went through all whitespace 
    // and reached the end
    lToken.type = TK_EOF;
    return lToken;
}


void destroy_LexToken(LexToken* lToken)
{
    if (lToken->type == TK_String_val || lToken->type == TK_Identifier || lToken->type == TK_UNKNOWN || lToken->type == TK_ERR)
        clearstring(&lToken->string);

    lToken->type   = TK_UNINIT;
    lToken->line   = 0;
    lToken->column = 0;
}


#endif