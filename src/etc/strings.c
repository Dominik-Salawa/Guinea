#ifndef STRINGS_C
#define STRINGS_C

#include "strings.h"
#include <string.h>

GUIN_String GUIN_init_String(void)
{
    GUIN_String s = (GUIN_String){.content=malloc(32),.length=0,.size=32};
    s.content[0] = 0;
    return s;
}

GUIN_String* GUIN_init_String_ptr(void)
{
    GUIN_String* x = malloc(sizeof(GUIN_String));
    if (!x) return NULL;
    *x = GUIN_init_String();
    return x;
}

GUIN_String* GUIN_stringaddchar(GUIN_String* str, char ch)
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

void GUIN_clearstring(GUIN_String* str)
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
void GUIN_clearstring_ptr(GUIN_String** str)
{
    if (!str) return;
    GUIN_clearstring(*str);
    free(*str);
    *str = NULL;
}

GUIN_String* GUIN_stringconcat(GUIN_String* toconcat, GUIN_String* toadd)
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

GUIN_String GUIN_copystring(GUIN_String* str)
{
    GUIN_String x;

    x.size    = str->size;
    x.length  = 0;
    x.content = malloc(x.size);
    if (!x.content) return (GUIN_String){0};

    GUIN_stringconcat(&x, str);
    x.content[x.length] = 0;
    return x;
}

GUIN_String* GUIN_copystring_as_ptr(GUIN_String* str)
{
    if (!str) return NULL;
    GUIN_String* x = GUIN_init_String_ptr();
    if (!x) return NULL;
    GUIN_stringconcat(x, str);
    return x;
}

// FOR RAW char* WITH A DEFINED length
GUIN_String* GUIN_stringconcat_char_w_len(GUIN_String* toconcat, const char* toadd, size_t toadd_len)
{
    if (toconcat->length + toadd_len + 1 > toconcat->size) {
        size_t newsize = toconcat->length + toadd_len + 1;
        char* tmp = realloc(toconcat->content, newsize);

        if (!tmp)
            return NULL;

        toconcat->size = newsize;
        toconcat->content = tmp;
    }

    for (size_t i = 0; i < toadd_len; i++) {
        toconcat->content[toconcat->length++] = toadd[i];
    }
    toconcat->content[toconcat->length] = 0;
    return toconcat;
}


GUIN_String* GUIN_stringconcat_charptr(GUIN_String* toconcat, const char* toadd)
{
    return GUIN_stringconcat_char_w_len(toconcat, toadd, strlen(toadd));
}

bool GUIN_stringcompare(GUIN_String* string1, GUIN_String* string2)
{
    if (string1->length != string2->length) return false;

    for (size_t i = 0; i < string1->length; i++) {
        if (string1->content[i] != string2->content[i]) return false;
    }

    return true;
}

#endif
