#ifndef STRING_MANIPULATION_C
#define STRING_MANIPULATION_C

#include "string_manipulation.h"
#include "../../include/declarations.h"
#include <stdbool.h>
#include <stdlib.h>

char GUIN_alphabet[] = "abcdefghijklmnopqrstuvwxyz";
char GUIN_numbers[]  = "0123456789";

unsigned char GUIN_to_hex_char(char ch)
{
    char low = GUIN_lower(ch);
    if (low >= 'a' && low <= 'f')
        ch = (low - 'a') + 10;
    else if (GUIN_is_num_char(ch))
        ch = GUIN_to_number(ch);

    return ch;
} 

bool GUIN_is_valid_identifier_char(char ch)
{
    if (ch == '_')       return true;
    if (GUIN_is_alphabet(ch)) return true;
    if (GUIN_is_num_char(ch)) return true;
    return false;
}

bool GUIN_is_int_string(char* str)
{
    size_t i = 0;
    for (; str[i] != 0; i++) {
        if (str[i] == '-' && i == 0)  continue;
        if (!GUIN_is_num_char(str[i])) return false;
    }
    return i != 0;
}

bool GUIN_is_valid_number(char* str)
{
    size_t i = 0;
    bool done_decimal = false;
    for (; str[i] != 0; i++) {
        if (str[i] == '-' && i == 0)        continue;

        if (str[i] == '.' && !done_decimal) {
            done_decimal = true;
            continue;
        }
        
        if (!GUIN_is_num_char(str[i]))       return false;
    }
    return i != 0;
}

bool GUIN_is_valid_identifier_string(char* str)
{
    bool found = false;
    for (GUIN_ubyte i = 0; i < sizeof(GUIN_alphabet)/sizeof(char)-1; i++) {
        if (GUIN_alphabet[i] == GUIN_lower(str[0]) || str[0] == '_') {
            found = true;
            break;
        }
    }
    if (!found) return false;

    size_t i = 1;
    for (; str[i] != 0; i++) {
        if (!GUIN_is_valid_identifier_char(str[i])) return false;
    }
    return true;
}

#endif
