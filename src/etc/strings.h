#ifndef STRINGS_H
#define STRINGS_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct GUIN_String {
    size_t length;
    size_t size;
    char* content;
} GUIN_String;

GUIN_String  GUIN_init_String(void);
GUIN_String* GUIN_init_String_ptr(void);
GUIN_String* GUIN_stringaddchar(GUIN_String* str, char ch);
// clearstring() IS IF YOU WANT TO FREE A STRING WITHOUT free() it
// OR IF ITS A STACK VALUE
void GUIN_clearstring(GUIN_String* str);
// clearstring_ptr() IS IF YOU WANT TO free() THE PTR STRING
void GUIN_clearstring_ptr(GUIN_String** str);
GUIN_String GUIN_copystring(GUIN_String* str);
GUIN_String* GUIN_copystring_as_ptr(GUIN_String* str);
GUIN_String* GUIN_stringconcat(GUIN_String* toconcat, GUIN_String* toadd);
// FOR RAW char* WITH A DEFINED length
GUIN_String* GUIN_stringconcat_char_w_len(GUIN_String* toconcat, const char* toadd, size_t length);
GUIN_String* GUIN_stringconcat_charptr(GUIN_String* toconcat, const char* toadd);
bool GUIN_stringcompare(GUIN_String* string1, GUIN_String* string2);

#endif
