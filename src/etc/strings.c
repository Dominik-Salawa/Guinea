#ifndef STRINGS_C
#define STRINGS_C

#include "strings.h"
#include <string.h>

String init_String()
{
    String s = (String){.content=malloc(32),.length=0,.size=32};
    s.content[0] = 0;
    return s;
}

String* init_String_ptr()
{
    String* x = malloc(sizeof(String));
    if (!x) return NULL;
    *x = init_String();
    return x;
}

String* stringaddchar(String* str, char ch)
{
    if (str->length >= str->size-1) {
        str->size *= 2;
        char* tmp = realloc(str->content, str->size);
        if (!tmp) return NULL;

        str->content = tmp;
    }
    str->content[str->length++] = ch;
    str->content[str->length]   = 0;
    return str;
}

void clearstring(String* str)
{
    if (!str) return;

    str->length = 0;
    str->size   = 0;

    if (str->content) {
        free(str->content);
        str->content = NULL;
    }
}

// THIS IS IF YOU WANT TO free() A PTR STRING
void clearstring_ptr(String** str)
{
    if (!str) return;
    clearstring(*str);
    free(*str);
    *str = NULL;
}

String* stringconcat(String* toconcat, String* toadd)
{
    while (toconcat->length + toadd->length + 1 > toconcat->size) {
        toconcat->size = toconcat->length + toadd->length + 1;
        char* tmp = realloc(toconcat->content, toconcat->size);

        if (!tmp)
            return NULL;

        toconcat->content = tmp;
    }

    for (size_t i = 0; i < toadd->length; i++)
        toconcat->content[toconcat->length++] = toadd->content[i];

    toconcat->content[toconcat->length] = 0;
    return toconcat;
}

String copystring(String* str)
{
    String x;

    x.size    = str->size;
    x.length  = 0;
    x.content = malloc(x.size);
    if (!x.content) return (String){0};

    stringconcat(&x, str);
    x.content[x.length] = 0;
    return x;
}

String* copystring_as_ptr(String* str)
{
    if (!str) return NULL;
    String* x = init_String_ptr();
    if (!x) return NULL;
    stringconcat(x, str);
    return x;
}

// FOR RAW char* WITH A DEFINED length
String* stringconcat_char_w_len(String* toconcat, char* toadd, size_t toadd_len)
{
    if (toconcat->length + toadd_len + 1 > toconcat->size) {
        toconcat->size = toconcat->length + toadd_len + 1;
        char* tmp = realloc(toconcat->content, toconcat->size);

        if (!tmp)
            return NULL;

        toconcat->content = tmp;
    }

    for (size_t i = 0; i < toadd_len; i++) {
        toconcat->content[toconcat->length++] = toadd[i];
    }
    toconcat->content[toconcat->length] = 0;
    return toconcat;
}


String* stringconcat_charptr(String* toconcat, char* toadd)
{
    return stringconcat_char_w_len(toconcat, toadd, strlen(toadd));
}

bool stringcompare(String* string1, String* string2)
{
    if (string1->length != string2->length) return false;

    for (size_t i = 0; i < string1->length; i++) {
        if (string1->content[i] != string2->content[i]) return false;
    }

    return true;
}

#endif