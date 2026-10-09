#ifndef guin_STRINGS_h
#define guin_STRINGS_h

#include <stdlib.h>
#include <stdbool.h>

typedef struct guin_String {
    size_t length;
    size_t size;
    char* content;
} guin_String;

guin_String  guin_init_String(void);
guin_String* guin_init_String_ptr(void);
guin_String* guin_stringaddchar(guin_String* str, char ch);
// clearstring() IS IF YOU WANT TO FREE A STRING WITHOUT free() it
// OR IF ITS A STACK VALUE
void guin_clearstring(guin_String* str);
// clearstring_ptr() IS IF YOU WANT TO free() THE PTR STRING
void guin_clearstring_ptr(guin_String** str);
guin_String guin_copystring(guin_String* str);
guin_String* guin_copystring_as_ptr(guin_String* str);
guin_String* guin_stringconcat(guin_String* toconcat, guin_String* toadd);
// FOR RAW char* WITH A DEFINED length
guin_String* guin_stringconcat_char_w_len(guin_String* toconcat, const char* toadd, size_t length);
guin_String* guin_stringconcat_charptr(guin_String* toconcat, const char* toadd);
bool guin_stringcompare(guin_String* string1, guin_String* string2);

#endif
