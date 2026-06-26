#ifndef LOG_H
#define LOG_H

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

/*

FORMATTING RULES FOR GUINEA PRINT

%z:     is size_t size, it applies to u, d, x, o like %zu or %zd etc...
%d:     fmt int
%o:     fmt octal
%x:     fmt hex
%s:     fmt Guinea String struct
%sc:    fmt char*

*/


size_t G_vprintf(FILE* file, char* string, va_list args);
size_t G_fprintf(FILE* file, char* string, ...);
size_t G_printf(char* string, ...);

size_t G_log_layer_size = 0;
bool G_log_on = false;
void G_log(char* string, ...);
void G_log_raw(char* string, ...);
void G_log_write(void* origin, size_t size, size_t number, FILE* fd);

#define G_log_pop_layer()  --G_log_layer_size
#define G_log_push_layer() ++G_log_layer_size

#endif