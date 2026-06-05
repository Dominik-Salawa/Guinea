#ifndef INPUT_C
#define INPUT_C

#include "strings.h"
#include "input.h"
#include <stdio.h>

String input()
{
    String str;
    str.length  = 0;
    str.size    = 32;
    str.content = malloc(32);

    int ch;
    while ((ch = fgetc(stdin)) > EOF && ch != 10) {
        if (str.length >= str.size-1) {
            str.size *= 2;
            str.content = realloc(str.content, str.size);

            if (!str.content) {
                str.size    = 0;
                str.length  = 0;
                return str;
            }
        }
        str.content[str.length++] = ch;
    }

    str.content[str.length] = 0;
    return str;
}

String readfile(FILE* f)
{
    String str;
    str.length  = 0;
    str.size    = 32;
    str.content = malloc(32);

    int ch;
    while ((ch = fgetc(f)) > EOF) {
        if (str.length >= str.size-1) {
            str.size *= 2;
            str.content = realloc(str.content, str.size);

            if (!str.content) {
                str.size    = 0;
                str.length  = 0;
                return str;
            }
        }
        str.content[str.length++] = ch;
    }

    str.content[str.length] = 0;
    return str;
}

#endif