#ifndef STRING_MANIPULATION_H
#define STRING_MANIPULATION_H

#include <stdlib.h>
#include <stdbool.h>

#define GUIN_is_num_char(ch)     (ch >= '0' && ch <= '9')
#define GUIN_is_octal_char(ch)   (ch >= '0' && ch <= '7')
#define GUIN_to_number(ch)       (ch - '0')
#define GUIN_is_alphabet(ch)     (GUIN_lower(ch) >= 'a' && GUIN_lower(ch) <= 'z')
#define GUIN_is_hex_code(ch)     ((GUIN_lower(ch) >= 'a' && GUIN_lower(ch) <= 'f') || GUIN_is_num_char(ch))
#define GUIN_lower(ch)           ((ch >= 'A' && ch <= 'Z')? ch + ('a'-'A') : ch)
#define GUIN_upper(ch)           ((ch >= 'a' && ch <= 'z')? ch - ('a'-'A') : ch)

unsigned char GUIN_to_hex_char(char ch);
bool GUIN_is_valid_identifier_char(char ch);
bool GUIN_is_int_string(char* str);
bool GUIN_is_valid_number(char* str);
bool GUIN_is_valid_identifier_string(char* str);

#endif
