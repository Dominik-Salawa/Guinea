#ifndef GUINEA_SLEEP_C
#define GUINEA_SLEEP_C

#include "sleep.h"
#include "../declarations.h"
#include <stdbool.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__unix__) || defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#else
#error "Undefined way of handling this platform for etc/stdlib/sleep.c!"
#endif

#include <stdio.h>
#include <stdlib.h>

bool G_sleep(G_uint32 miliseconds)
{
    #if defined(_WIN32)
    Sleep(miliseconds);
    #elif defined(__unix__) || defined(__APPLE__) || defined(__linux__)
    usleep(miliseconds * 1000);
    #else
    #error "Undefined way of handling this platform for etc/stdlib/sleep.c/G_sleep()!"
    #endif
    return true; // so it can be used in loops like `while sleep(100) do ... end`
}

#endif