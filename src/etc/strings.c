#ifndef STRINGS_C
#define STRINGS_C

#include "strings.h"

String init_String()
{
    String s = (String){.content=malloc(32),.length=0,.size=32};
    s.content[0] = 0;
    return s;
}

void stringaddchar(String* str, char ch)
{
    if (str->length >= str->size-1) {
        str->size *= 2;
        str->content = realloc(str->content, str->size);

        if (!str->content) {
            str->size    = 0;
            str->length  = 0;
        }
    }
    str->content[str->length++] = ch;
    str->content[str->length]   = 0;
}

void clearstring(String* str)
{
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
    clearstring(*str);
    free(*str);
    *str = NULL;
}

String copystring(String* str)
{
    String x;

    x.size    = str->size;
    x.length  = str->length;
    x.content = malloc(x.size);

    for (size_t i = 0; i < x.length; i++)
        x.content[i] = str->content[i];

    x.content[x.length] = 0;

    return x;
}

bool stringconcat(String* toconcat, String* toadd)
{
    if (toconcat->length + toadd->length + 1 > toconcat->size) {
        toconcat->size = toconcat->length + toadd->length + 1;
        char* tmp = realloc(toconcat->content, toconcat->size);

        if (!tmp)
            return false;

        toconcat->content = tmp;
    }

    size_t toconcat_i = toconcat->length;
    for (size_t i = 0; i < toadd->length; i++) {
        toconcat->content[toconcat_i] = toadd->content[i];
        toconcat_i++;
    }
    toconcat->length = toconcat->length + toadd->length;
    toconcat->content[toconcat->length] = 0;
    return true;
}

bool stringconcat_charptr(String* toconcat, char* toadd)
{
    size_t toadd_len = 0;
    while (toadd[toadd_len] != 0) toadd_len++;

    if (toconcat->length + toadd_len + 1 > toconcat->size) {
        toconcat->size = toconcat->length + toadd_len + 1;
        char* tmp = realloc(toconcat->content, toconcat->size);

        if (!tmp)
            return false;

        toconcat->content = tmp;
    }

    size_t toconcat_i = toconcat->length;
    for (size_t i = 0; i < toadd_len; i++) {
        toconcat->content[toconcat_i] = toadd[i];
        toconcat_i++;
    }
    toconcat->length = toconcat->length + toadd_len;
    toconcat->content[toconcat->length] = 0;
    return true;
}

#endif