#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <inttypes.h>
typedef unsigned char uchar;
typedef int8_t        byte;
typedef uint8_t       ubyte;

typedef int16_t       int16;
typedef uint16_t      uint16;
typedef int32_t       int32;
typedef uint32_t      uint32;
typedef int64_t       int64;
typedef uint64_t      uint64;

typedef float         number32;
typedef double        number64;
typedef void (*RawFuncPointer)(void*);

number32 __G_NaN()
{
    union {
        uint32 i;
        number32 n;
    } u;

    u.i = 0x7FF80000;
    return u.n;
}

#define G_NaN       (__G_NaN())
#define is_NaN(num) (num != num) // according to IEEE 754, a NaN cannot equ to itself

#endif