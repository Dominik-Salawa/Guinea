#ifndef GUINEA_STDIO_H
#define GUINEA_STDIO_H

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

/*

FORMATTING RULES FOR GUINEA PRINT

%z:     is size_t size, it applies to u, d, x, o like %zu or %zd etc...
%d:     fmt int
%f:     fmt float
%o:     fmt octal
%x:     fmt hex
%s:     fmt Guinea String struct
%sc:    fmt char*

*/


size_t G_vprintf(FILE* file, char* string, va_list args);
size_t G_fprintf(FILE* file, char* string, ...);
size_t G_printf(char* string, ...);

String input();
String readfile(FILE* file);

#endif