#ifndef LOG_C
#define LOG_C

#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "log.h"

void G_log(char* string, ...)
{
    if (!G_log_on) return;

    for (size_t i = 0; i < G_log_layer_size; i++) {
        putchar('\t');
    }

    va_list list_of_args;
    va_start(list_of_args, string);
    vprintf(string, list_of_args);
    va_end(list_of_args);
}

void G_log_write(void* origin, size_t size, size_t number, FILE* fd)
{
    if (!G_log_on) return;
    fwrite(origin, size, number, fd);
}

#endif