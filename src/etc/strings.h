#ifndef STRINGS_H
#define STRINGS_H

#if defined(__cplusplus)
#extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>

typedef struct String {
    size_t length;
    size_t size;
    char* content;
} String;

String init_String();
String* init_String_ptr();
String* stringaddchar(String* str, char ch);
// clearstring() IS IF YOU WANT TO FREE A STRING WITHOUT free() it
// OR IF ITS A STACK VALUE
void clearstring(String* str);
// clearstring_ptr() IS IF YOU WANT TO free() THE PTR STRING
void clearstring_ptr(String** str);
String copystring(String* str);
String* copystring_as_ptr(String* str);
String* stringconcat(String* toconcat, String* toadd);
// FOR RAW char* WITH A DEFINED length
String* stringconcat_char_w_len(String* toconcat, char* toadd, size_t length);
String* stringconcat_charptr(String* toconcat, char* toadd);
bool stringcompare(String* string1, String* string2);

#if defined(__cplusplus)
}
#endif

#endif