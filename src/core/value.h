#ifndef VALUE_H
#define VALUE_H

#include <inttypes.h>
#include <stdlib.h>
#include "../etc/strings.h"

typedef enum {
    G_VALUETYPE_DEAD = 0, // GC

    G_VALUETYPE_CHAR,
    G_VALUETYPE_STRING,

    G_VALUETYPE_NIL,

    G_VALUETYPE_BOOL,

    G_VALUETYPE_F32,
    G_VALUETYPE_F64,

    G_VALUETYPE_I32,
    G_VALUETYPE_I64,
} G_ValueType;


typedef struct G_Value G_Value;
typedef struct {
    uint64_t size;
    G_Value** children;
} G_Childrens;
struct G_Value {
    G_ValueType type;
    union {
        bool            bl;
        char            ch;
        String          str;
        float           f32;
        double          f64;
        int32_t         i32;
        int64_t         i64;
        G_Childrens     children;
    };
};

G_Value init_Value();

#endif