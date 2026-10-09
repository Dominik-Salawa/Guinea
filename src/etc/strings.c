#ifndef guin_STRINGS_c
#define guin_STRINGS_c

#include "strings.h"
#include <string.h>

guin_String guin_init_String(void)
{
    guin_String s = (guin_String){.content=malloc(32),.length=0,.size=32};
    s.content[0] = 0;
    return s;
}

guin_String* guin_init_String_ptr(void)
{
    guin_String* x = malloc(sizeof(guin_String));
    if (!x) return NULL;
    *x = guin_init_String();
    return x;
}

guin_String* guin_stringaddchar(guin_String* str, char ch)
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

void guin_clearstring(guin_String* str)
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
void guin_clearstring_ptr(guin_String** str)
{
    if (!str) return;
    guin_clearstring(*str);
    free(*str);
    *str = NULL;
}

guin_String* guin_stringconcat(guin_String* toconcat, guin_String* toadd)
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

guin_String guin_copystring(guin_String* str)
{
    guin_String x;

    x.size    = str->size;
    x.length  = 0;
    x.content = malloc(x.size);
    if (!x.content) return (guin_String){0};

    guin_stringconcat(&x, str);
    x.content[x.length] = 0;
    return x;
}

guin_String* guin_copystring_as_ptr(guin_String* str)
{
    if (!str) return NULL;
    guin_String* x = guin_init_String_ptr();
    if (!x) return NULL;
    guin_stringconcat(x, str);
    return x;
}

// FOR RAW char* WITH A DEFINED length
guin_String* guin_stringconcat_char_w_len(guin_String* toconcat, const char* toadd, size_t toadd_len)
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


guin_String* guin_stringconcat_charptr(guin_String* toconcat, const char* toadd)
{
    return guin_stringconcat_char_w_len(toconcat, toadd, strlen(toadd));
}

bool guin_stringcompare(guin_String* string1, guin_String* string2)
{
    if (string1->length != string2->length) return false;

    for (size_t i = 0; i < string1->length; i++) {
        if (string1->content[i] != string2->content[i]) return false;
    }

    return true;
}

#endif
