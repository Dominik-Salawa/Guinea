#ifndef GUINEA_STDIO_H
#define GUINEA_STDIO_H

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


size_t GUIN_vprintf(FILE* file, char* string, va_list args);
size_t GUIN_fprintf(FILE* file, char* string, ...);
size_t GUIN_printf(char* string, ...);

GUIN_String GUIN_input(void);
GUIN_String GUIN_readfile(FILE* file);

#endif
