#ifndef guin_LOG_c
#define guin_LOG_c

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include "log.h"

size_t guin_log_layer_size = 0;
bool guin_log_on = false;

void guin_log(char* string, ...)
{
    if (!guin_log_on) return;

    for (size_t i = 0; i < guin_log_layer_size; i++) {
        putchar('\t');
    }

    va_list args;
    va_start(args, string);
    vprintf(string, args);
    va_end(args);
}

void guin_log_raw(char* string, ...)
{
    if (!guin_log_on) return;

    va_list args;
    va_start(args, string);
    vprintf(string, args);
    va_end(args);
}

void guin_log_write(void* origin, size_t size, size_t number, FILE* fd)
{
    if (!guin_log_on) return;
    fwrite(origin, size, number, fd);
}

#endif
