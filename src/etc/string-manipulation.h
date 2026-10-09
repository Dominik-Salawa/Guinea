#ifndef guin_STRING_MANIPULATION_H
#define guin_STRING_MANIPULATION_H

#include <stdlib.h>
#include <stdbool.h>

#define guin_is_num_char(ch)     (ch >= '0' && ch <= '9')
#define guin_is_octal_char(ch)   (ch >= '0' && ch <= '7')
#define guin_to_number(ch)       (ch - '0')
#define guin_is_alphabet(ch)     (guin_lower(ch) >= 'a' && guin_lower(ch) <= 'z')
#define guin_is_hex_code(ch)     ((guin_lower(ch) >= 'a' && guin_lower(ch) <= 'f') || guin_is_num_char(ch))
#define guin_lower(ch)           ((ch >= 'A' && ch <= 'Z')? ch + ('a'-'A') : ch)
#define guin_upper(ch)           ((ch >= 'a' && ch <= 'z')? ch - ('a'-'A') : ch)

unsigned char guin_to_hex_char(char ch);
bool guin_is_valid_identifier_char(char ch);
bool guin_is_int_string(char* str);
bool guin_is_valid_number(char* str);
bool guin_is_valid_identifier_string(char* str);

#endif
