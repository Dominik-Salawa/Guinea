#ifndef guin_STRING_MANIPULATION_c
#define guin_STRING_MANIPULATION_c

#include "string-manipulation.h"
#include "../../include/declarations.h"
#include <stdbool.h>
#include <stdlib.h>

char guin_alphabet[] = "abcdefghijklmnopqrstuvwxyz";
char guin_numbers[]  = "0123456789";

unsigned char guin_to_hex_char(char ch)
{
    char low = guin_lower(ch);
    if (low >= 'a' && low <= 'f')
        ch = (low - 'a') + 10;
    else if (guin_is_num_char(ch))
        ch = guin_to_number(ch);

    return ch;
} 

bool guin_is_valid_identifier_char(char ch)
{
    if (ch == '_')       return true;
    if (guin_is_alphabet(ch)) return true;
    if (guin_is_num_char(ch)) return true;
    return false;
}

bool guin_is_int_string(char* str)
{
    size_t i = 0;
    for (; str[i] != 0; i++) {
        if (str[i] == '-' && i == 0)  continue;
        if (!guin_is_num_char(str[i])) return false;
    }
    return i != 0;
}

bool guin_is_valid_number(char* str)
{
    size_t i = 0;
    bool done_decimal = false;
    for (; str[i] != 0; i++) {
        if (str[i] == '-' && i == 0)        continue;

        if (str[i] == '.' && !done_decimal) {
            done_decimal = true;
            continue;
        }
        
        if (!guin_is_num_char(str[i]))       return false;
    }
    return i != 0;
}

bool guin_is_valid_identifier_string(char* str)
{
    bool found = false;
    for (guin_ubyte i = 0; i < sizeof(guin_alphabet)/sizeof(char)-1; i++) {
        if (guin_alphabet[i] == guin_lower(str[0]) || str[0] == '_') {
            found = true;
            break;
        }
    }
    if (!found) return false;

    size_t i = 1;
    for (; str[i] != 0; i++) {
        if (!guin_is_valid_identifier_char(str[i])) return false;
    }
    return true;
}

#endif
