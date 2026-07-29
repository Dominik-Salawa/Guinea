#ifndef GUIN_H
#define GUIN_H

#if defined(__cplusplus)
extern "C" {
#endif

// /src/guinea.c DOES NOT SERVE THE SAME PURPOSE AND IT IS JUST FOR EMBEDDING API
// /include/guinea.h DOES NOT SERVE THE SAME PURPOSE AND IT IS JUST FOR EMBEDDING API
// guinea.c is located in /src/

#include <stdbool.h>
#include <stdio.h>
#include "declarations.h"

typedef enum GUIN_API_Value_type {
    GUIN_API_DT_NIL = 0,
    GUIN_API_DT_I16,
    GUIN_API_DT_I32,
    GUIN_API_DT_I64,
    GUIN_API_DT_N32,
    GUIN_API_DT_N64,
    GUIN_API_DT_CHAR,
    GUIN_API_DT_BOOL,
    GUIN_API_DT_STR,
} GUIN_API_Value_type;

typedef struct GUIN_API_Value {
    GUIN_API_Value_type type;
    union {
        GUIN_int16 i16;
        GUIN_int32 i32;
        GUIN_int64 i64;
        GUIN_number32 n32;
        GUIN_number64 n64;
        char ch;
        bool bl;
        const char* str;
    };
} GUIN_API_Value;

struct GUIN_nil__struct__;
#define GUIN_nil ((struct GUIN_nil__struct__*)0)

GUIN_API_Value GUIN_API_nil(void* _);
GUIN_API_Value GUIN_API_int16(GUIN_int16 data);
GUIN_API_Value GUIN_API_int32(GUIN_int32 data);
GUIN_API_Value GUIN_API_int64(GUIN_int64 data);
GUIN_API_Value GUIN_API_number32(GUIN_number32 data);
GUIN_API_Value GUIN_API_number64(GUIN_number64 data);
GUIN_API_Value GUIN_API_char(char data);
GUIN_API_Value GUIN_API_bool(bool data);
GUIN_API_Value GUIN_API_string(const char* data);



#if !defined(__cplusplus)
    #define GUIN_VALUE(value) (_Generic((value),\
        void*:         GUIN_API_nil,\
        struct GUIN_nil__struct__*: GUIN_API_nil,\
        GUIN_int16:    GUIN_API_int16,\
        GUIN_int32:    GUIN_API_int32,\
        GUIN_int64:    GUIN_API_int64,\
        GUIN_number32: GUIN_API_number32,\
        GUIN_number64: GUIN_API_number64,\
        bool:          GUIN_API_bool,\
        char:          GUIN_API_char,\
        char*:         GUIN_API_string,\
        const char*:   GUIN_API_string\
    )(value))
#else
} // FOR EXTERN C

    #include <cstddef> // for nullptr_t

    GUIN_API_Value GUIN_VALUE(GUIN_int16 value)
        { return GUIN_API_int16(value); }
    GUIN_API_Value GUIN_VALUE(GUIN_int32 value)
        { return GUIN_API_int32(value); }
    GUIN_API_Value GUIN_VALUE(long long value)
        { return GUIN_API_int64(value); }
    GUIN_API_Value GUIN_VALUE(GUIN_int64 value)
        { return GUIN_API_int64(value); }

    GUIN_API_Value GUIN_VALUE(GUIN_number32 value)
        { return GUIN_API_number32(value); }
    GUIN_API_Value GUIN_VALUE(GUIN_number64 value)
        { return GUIN_API_number64(value); }
    
    GUIN_API_Value GUIN_VALUE(bool value)
        { return GUIN_API_bool(value); }
    GUIN_API_Value GUIN_VALUE(char value)
        { return GUIN_API_char(value); }
    GUIN_API_Value GUIN_VALUE(char* value)
        { return GUIN_API_string(value); }
    GUIN_API_Value GUIN_VALUE(const char* value)
        { return GUIN_API_string(value); }

    GUIN_API_Value GUIN_VALUE(std::nullptr_t value)
        { return GUIN_API_nil(nullptr); }
    GUIN_API_Value GUIN_VALUE(void* value)
        { return GUIN_API_nil(value); }
    GUIN_API_Value GUIN_VALUE(struct GUIN_nil__struct__* value)
        { return GUIN_API_nil(value); }

extern "C" {
#endif

typedef struct GUIN_VM GUIN_VM;
GUIN_VM* GUIN_API_open_VM(void);
void GUIN_API_print_main_stack(GUIN_VM* vm);
void GUIN_API_print_global(GUIN_VM* vm);
//bool GUIN_API_load_VM_w_file(GUIN_VM* vm, FILE* file);
//bool GUIN_API_load_VM_w_dir(GUIN_VM* vm, const char* dir);
//const char* GUIN_API_extract_error(GUIN_VM* vm);
void GUIN_API_close_VM(GUIN_VM** vm);

//bool GUIN_API_declare_global_VM(GUIN_VM* vm, const char* global_name);

//bool GUIN_API_load_global(GUIN_VM* vm, const char* global_name);
//bool GUIN_API_load_field(GUIN_VM* vm);
//bool GUIN_API_load_index(GUIN_VM* vm, GUIN_API_Value value);
//bool GUIN_API_push_load(GUIN_VM* vm);
//bool GUIN_API_write_to_load(GUIN_VM* vm);

// used in order to signify when the end of the function args are in the stack
bool GUIN_API_push_IMMEDIATE(GUIN_VM* vm, GUIN_API_Value value);
//bool GUIN_API_push_GLOBAL(GUIN_VM* vm, const char* global_name);

// pops 2 vals, returns 1 value
bool GUIN_API_ADD(GUIN_VM* vm);
bool GUIN_API_SUB(GUIN_VM* vm);
bool GUIN_API_MUL(GUIN_VM* vm);
bool GUIN_API_DIV(GUIN_VM* vm);
bool GUIN_API_MOD(GUIN_VM* vm);
bool GUIN_API_POW(GUIN_VM* vm);

bool GUIN_API_AND(GUIN_VM* vm);
bool GUIN_API_OR(GUIN_VM* vm);
bool GUIN_API_EQU(GUIN_VM* vm);
bool GUIN_API_NOT_EQU(GUIN_VM* vm);
bool GUIN_API_LT(GUIN_VM* vm);
bool GUIN_API_GT(GUIN_VM* vm);
bool GUIN_API_LT_EQU(GUIN_VM* vm);
bool GUIN_API_GT_EQU(GUIN_VM* vm);

// makes the top value the opposite of what it is
bool GUIN_API_NOT(GUIN_VM* vm);
// makes the top value opposite of the sign
bool GUIN_API_NEG(GUIN_VM* vm);

// you have to push all the function args correctly, pops 1 (for func)
//bool GUIN_API_FUNC_CALL(GUIN_VM* vm, GUIN_ubyte argc);
// does the same as GUIN_API_FUNC_CALL, but it guarantees it wont push 
// something onto the stack once done (the ret value)
//bool GUIN_API_FUNC_CALL_NO_RET(GUIN_VM* vm, GUIN_ubyte argc);

#if defined(__cplusplus)
}
#endif

#endif
