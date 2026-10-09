#ifndef guin_STDIO_h
#define guin_STDIO_h

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include "../strings.h"

/*

FORMATTING RULES FOR GUINEA PRINT

%z:     is size_t size, it applies to u, d, x, o like %zu or %zd etc...
%d:     fmt int
%f:     fmt float
%o:     fmt octal
%x:     fmt hex
%p:     fmt pointer
%s:     fmt Guinea String pointer
%sc:    fmt char*

*/


long guin_vprintf(FILE* file, char* string, va_list args);
long guin_fprintf(FILE* file, char* string, ...);
long guin_printf(char* string, ...);

guin_String guin_input(void);
guin_String guin_readfile(FILE* file);

#endif
