#ifndef guin_DECLARATIONS_h
#define guin_DECLARATIONS_h

#include <inttypes.h>
#include <math.h>

typedef unsigned char guin_uchar;

typedef int8_t        guin_byte;
typedef uint8_t       guin_ubyte;

typedef int16_t       guin_int16;
typedef uint16_t      guin_uint16;
typedef int32_t       guin_int32;
typedef uint32_t      guin_uint32;
typedef int64_t       guin_int64;
typedef uint64_t      guin_uint64;

typedef float         guin_number32;
typedef double        guin_number64;
typedef void (*RawFuncPointer)(void*);

typedef guin_uint16 guin_LOCAL_SLOT_INT;
typedef guin_uint32 guin_JUMPL_SIZE;

typedef enum guin_STATUS {
    guin_SUCCESS = 0, guin_FAIL, guin_MEM_FAIL, guin_CONTINUE, guin_FINISHED, guin_CORRUPTED_DATA
} guin_STATUS;

guin_number32 guin__NaN__(void);

#define guin_Inf         (INFINITY)
#define guin_NaN         (guin__NaN__())
#define guin_is_NaN(num) (num != num) // according to IEEE 754, a NaN cannot equ to itself

void* guin_malloc(size_t size);
void* guin_remalloc(void* ptr, size_t size);
void  guin_mfree(void* ptr);

#endif
