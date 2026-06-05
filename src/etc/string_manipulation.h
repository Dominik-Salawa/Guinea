#ifndef STRING_MANIPULATION_H
#define STRING_MANIPULATION_H

#include <stdlib.h>
#include <stdbool.h>

#define is_num_char(ch)     (ch >= '0' && ch <= '9')
#define is_octal_char(ch)   (ch >= '0' && ch <= '7')
#define to_number(ch)       (ch - '0')
#define is_alphabet(ch)     (lower(ch) >= 'a' && lower(ch) <= 'z')
#define is_hex_code(ch)     (lower(ch) >= 'a' && lower(ch) <= 'f' || is_num_char(ch))

char lower(char ch);
unsigned char to_hex_char(char ch);
bool is_valid_identifier_char(char ch);
bool is_int_string(char* str);
bool is_valid_number(char* str);
bool is_valid_identifier_string(char* str);

#endif