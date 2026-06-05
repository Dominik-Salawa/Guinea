#ifndef STRING_MANIPULATION_C
#define STRING_MANIPULATION_C

#include "string_manipulation.h"
#include <stdbool.h>
#include <stdlib.h>

#define is_num_char(ch)     (ch >= '0' && ch <= '9')
#define is_octal_char(ch)   (ch >= '0' && ch <= '7')
#define to_number(ch)       (ch - '0')
#define is_alphabet(ch)     (lower(ch) >= 'a' && lower(ch) <= 'z')
#define is_hex_code(ch)     (lower(ch) >= 'a' && lower(ch) <= 'f' || is_num_char(ch))

char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
char numbers[]  = "0123456789";

char lower(char ch)
{
    if (ch >= 'A' && ch <= 'Z') ch += ('a' - 'A');
    return ch;
}


unsigned char to_hex_char(char ch)
{
    char low = lower(ch);
    if (low >= 'a' && low <= 'f')
        ch = (low - 'a') + 10;
    else if (is_num_char(ch))
        ch = to_number(ch);

    return ch;
} 

bool is_valid_identifier_char(char ch)
{
    if (ch == '_')       return true;
    if (is_alphabet(ch)) return true;
    if (is_num_char(ch)) return true;
    return false;
}

bool is_int_string(char* str)
{
    size_t i = 0;
    for (; str[i] != 0; i++) {
        if (str[i] == '-' && i == 0)  continue;
        if (!is_num_char(str[i])) return false;
    }
    return i != 0;
}

bool is_valid_number(char* str)
{
    size_t i = 0;
    bool done_decimal = false;
    for (; str[i] != 0; i++) {
        if (str[i] == '-' && i == 0)        continue;

        if (str[i] == '.' && !done_decimal) {
            done_decimal = true;
            continue;
        }
        
        if (!is_num_char(str[i]))       return false;
    }
    return i != 0;
}

bool is_valid_identifier_string(char* str)
{
    bool found = false;
    for (char i = 0; i < sizeof(alphabet)/sizeof(char)-1; i++) {
        if (alphabet[i] == lower(str[0]) || str[0] == '_') {
            found = true;
            break;
        }
    }
    if (!found) return false;

    size_t i = 1;
    for (; str[i] != 0; i++) {
        if (!is_valid_identifier_char(str[i])) return false;
    }
    return true;
}

#endif