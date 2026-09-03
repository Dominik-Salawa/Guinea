#ifndef GUIN_LOG_C
#define GUIN_LOG_C

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include "log.h"

size_t GUIN_log_layer_size = 0;
bool GUIN_log_on = false;

void GUIN_log(char* string, ...)
{
    if (!GUIN_log_on) return;

    for (size_t i = 0; i < GUIN_log_layer_size; i++) {
        putchar('\t');
    }

    va_list args;
    va_start(args, string);
    vprintf(string, args);
    va_end(args);
}

void GUIN_log_raw(char* string, ...)
{
    if (!GUIN_log_on) return;

    va_list args;
    va_start(args, string);
    vprintf(string, args);
    va_end(args);
}

void GUIN_log_write(void* origin, size_t size, size_t number, FILE* fd)
{
    if (!GUIN_log_on) return;
    fwrite(origin, size, number, fd);
}

#endif
