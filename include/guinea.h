#ifndef guin_API_h
#define guin_API_h

#if defined(__cplusplus)
extern "C" {
#endif

// /src/guinea.c DOES NOT SERVE THE SAME PURPOSE AND IT IS JUST FOR EMBEDDING API
// /include/guinea.h DOES NOT SERVE THE SAME PURPOSE AND IT IS JUST FOR EMBEDDING API
// guinea.c is located in /src/

#include <stdbool.h>
#include <stdio.h>
#include "declarations.h"

typedef enum guin_API_Value_type {
    guin_API_DT_NIL = 0,
    guin_API_DT_I16,
    guin_API_DT_I32,
    guin_API_DT_I64,
    guin_API_DT_N32,
    guin_API_DT_N64,
    guin_API_DT_CHAR,
    guin_API_DT_BOOL,
    guin_API_DT_STR,
} guin_API_Value_type;

typedef struct guin_API_Value {
    guin_API_Value_type type;
    union {
        guin_int16 i16;
        guin_int32 i32;
        guin_int64 i64;
        guin_number32 n32;
        guin_number64 n64;
        char ch;
        bool bl;
        const char* str;
    };
} guin_API_Value;

struct guin_nil__struct__;
#define guin_nil ((struct guin_nil__struct__*)0)

guin_API_Value guin_API_nil(void* _);
guin_API_Value guin_API_int16(guin_int16 data);
guin_API_Value guin_API_int32(guin_int32 data);
guin_API_Value guin_API_int64(guin_int64 data);
guin_API_Value guin_API_number32(guin_number32 data);
guin_API_Value guin_API_number64(guin_number64 data);
guin_API_Value guin_API_char(char data);
guin_API_Value guin_API_bool(bool data);
guin_API_Value guin_API_string(const char* data);



#if !defined(__cplusplus)
    #define guin_VALUE(value) (_Generic((value),\
        void*:         guin_API_nil,\
        struct guin_nil__struct__*: guin_API_nil,\
        guin_int16:    guin_API_int16,\
        guin_int32:    guin_API_int32,\
        guin_int64:    guin_API_int64,\
        guin_number32: guin_API_number32,\
        guin_number64: guin_API_number64,\
        bool:          guin_API_bool,\
        char:          guin_API_char,\
        char*:         guin_API_string,\
        const char*:   guin_API_string\
    )(value))
#else
} // FOR EXTERN C

    #include <cstddef> // for nullptr_t

    inline guin_API_Value guin_VALUE(guin_int16 value)
        { return guin_API_int16(value); }
    inline guin_API_Value guin_VALUE(guin_int32 value)
        { return guin_API_int32(value); }
    inline guin_API_Value guin_VALUE(long long value)
        { return guin_API_int64(value); }
    inline guin_API_Value guin_VALUE(guin_int64 value)
        { return guin_API_int64(value); }

    inline guin_API_Value guin_VALUE(guin_number32 value)
        { return guin_API_number32(value); }
    inline guin_API_Value guin_VALUE(guin_number64 value)
        { return guin_API_number64(value); }
    
    inline guin_API_Value guin_VALUE(bool value)
        { return guin_API_bool(value); }
    inline guin_API_Value guin_VALUE(char value)
        { return guin_API_char(value); }
    inline guin_API_Value guin_VALUE(char* value)
        { return guin_API_string(value); }
    inline guin_API_Value guin_VALUE(const char* value)
        { return guin_API_string(value); }

    inline guin_API_Value guin_VALUE(struct guin_nil__struct__* value)
        { return guin_API_nil(value); }
    inline guin_API_Value guin_VALUE(std::nullptr_t value)
        { return guin_API_nil(nullptr); }
    inline guin_API_Value guin_VALUE(void* value)
        { return guin_API_nil(value); }

extern "C" {
#endif



typedef struct guin_VM guin_VM;
guin_VM* guin_API_open_VM(void);
void guin_API_print_main_stack(guin_VM* vm);
void guin_API_print_global(guin_VM* vm);
//bool guin_API_load_VM_w_file(guin_VM* vm, FILE* file);
//bool guin_API_load_VM_w_dir(guin_VM* vm, const char* dir);
//const char* guin_API_extract_error(guin_VM* vm);
void guin_API_close_VM(guin_VM** vm);

//bool guin_API_declare_global_VM(guin_VM* vm, const char* global_name);

//bool guin_API_load_global(guin_VM* vm, const char* global_name);
//bool guin_API_load_field(guin_VM* vm);
//bool guin_API_load_index(guin_VM* vm, guin_API_Value value);
//bool guin_API_push_load(guin_VM* vm);
//bool guin_API_write_to_load(guin_VM* vm);

// used in order to signify when the end of the function args are in the stack
bool guin_API_push_IMMEDIATE(guin_VM* vm, guin_API_Value value);
//bool guin_API_push_GLOBAL(guin_VM* vm, const char* global_name);

// pops 2 vals, returns 1 value
bool guin_API_ADD(guin_VM* vm);
bool guin_API_SUB(guin_VM* vm);
bool guin_API_MUL(guin_VM* vm);
bool guin_API_DIV(guin_VM* vm);
bool guin_API_MOD(guin_VM* vm);
bool guin_API_POW(guin_VM* vm);

bool guin_API_AND(guin_VM* vm);
bool guin_API_OR(guin_VM* vm);
bool guin_API_EQU(guin_VM* vm);
bool guin_API_NOT_EQU(guin_VM* vm);
bool guin_API_LT(guin_VM* vm);
bool guin_API_GT(guin_VM* vm);
bool guin_API_LT_EQU(guin_VM* vm);
bool guin_API_GT_EQU(guin_VM* vm);

// makes the top value the opposite of what it is
bool guin_API_NOT(guin_VM* vm);
// makes the top value opposite of the sign
bool guin_API_NEG(guin_VM* vm);

// you have to push all the function args correctly, pops 1 (for func)
//bool guin_API_FUNC_CALL(guin_VM* vm, guin_ubyte argc);
// does the same as guin_API_FUNC_CALL, but it guarantees it wont push 
// something onto the stack once done (the ret value)
//bool guin_API_FUNC_CALL_NO_RET(guin_VM* vm, guin_ubyte argc);

#if defined(__cplusplus)
}
#endif

#endif
