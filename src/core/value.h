#ifndef VALUE_H
#define VALUE_H

#include <inttypes.h>
#include <stdlib.h>
#include "../etc/strings.h"

typedef enum G_ValueType {
    G_VALUETYPE_DEAD = 0,

    G_VALUETYPE_CHAR,
    G_VALUETYPE_STRING,

    G_VALUETYPE_NIL,

    G_VALUETYPE_BOOL,

    G_VALUETYPE_F32,
    G_VALUETYPE_F64,

    G_VALUETYPE_I32,
    G_VALUETYPE_I64,
} G_ValueType;


typedef struct G_Value {
    G_ValueType type;
    union {
        bool            bl;
        char            ch;
        String          str;
        float           f32;
        double          f64;
        int32_t         i32;
        int64_t         i64;
        struct {
            size_t size;
            struct G_Value** children;
        } children;
    };
} G_Value;

G_Value init_Value();

#endif