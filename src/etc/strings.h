#ifndef STRINGS_H
#define STRINGS_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    size_t length;
    size_t size;
    char* content;
} String;

String init_String();
void stringaddchar(String* str, char ch);
// clearstring() IS IF YOU WANT TO FREE A STRING WITHOUT free() it
// OR IF ITS A STACK VALUE
void clearstring(String* str);
// clearstring_ptr() IS IF YOU WANT TO free() THE PTR STRING
void clearstring_ptr(String** str);
String copystring(String* str);
bool stringconcat(String* toconcat, String* toadd);
bool stringconcat_charptr(String* toconcat, char* toadd);

#endif