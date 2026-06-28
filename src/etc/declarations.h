#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <inttypes.h>
typedef unsigned char G_uchar;

typedef int8_t        G_byte;
typedef uint8_t       G_ubyte;

typedef int16_t       G_int16;
typedef uint16_t      G_uint16;
typedef int32_t       G_int32;
typedef uint32_t      G_uint32;
typedef int64_t       G_int64;
typedef uint64_t      G_uint64;

typedef float         G_number32;
typedef double        G_number64;
typedef void (*RawFuncPointer)(void*);

G_number32 __G_NaN()
{
    union {
        G_uint32 i;
        G_number32 n;
    } u;

    u.i = 0x7FF80000;
    return u.n;
}

#define G_NaN       (__G_NaN())
#define is_NaN(num) (num != num) // according to IEEE 754, a NaN cannot equ to itself

#endif