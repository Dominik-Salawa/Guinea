#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <inttypes.h>
#include <math.h>

typedef unsigned char GUIN_uchar;

typedef int8_t        GUIN_byte;
typedef uint8_t       GUIN_ubyte;

typedef int16_t       GUIN_int16;
typedef uint16_t      GUIN_uint16;
typedef int32_t       GUIN_int32;
typedef uint32_t      GUIN_uint32;
typedef int64_t       GUIN_int64;
typedef uint64_t      GUIN_uint64;

typedef float         GUIN_number32;
typedef double        GUIN_number64;
typedef void (*RawFuncPointer)(void*);

typedef GUIN_uint16 GUIN_LOCAL_SLOT_INT;
typedef GUIN_uint32 GUIN_JUMPL_SIZE;

typedef enum GUIN_STATUS {
    GUIN_SUCCESS = 0, GUIN_FAIL, GUIN_MEM_FAIL, GUIN_CONTINUE, GUIN_FINISHED
} GUIN_STATUS;

GUIN_number32 GUIN__NaN__(void);

#define GUIN_Inf         (INFINITY)
#define GUIN_NaN         (GUIN__NaN__())
#define GUIN_is_NaN(num) (num != num) // according to IEEE 754, a NaN cannot equ to itself

#endif
