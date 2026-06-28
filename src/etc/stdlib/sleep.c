#ifndef GUINEA_SLEEP_C
#define GUINEA_SLEEP_C

#include "sleep.h"
#include <stdbool.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__unix__) || defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>

bool G_sleep(int miliseconds)
{
    #if defined(_WIN32)
    Sleep(miliseconds);
    #elif defined(__unix__) || defined(__APPLE__) || defined(__linux__)
    usleep(miliseconds * 1000);
    #else
    #error "Undefined way of handling this platform for sleep!"
    #endif

    return true;
}

#endif