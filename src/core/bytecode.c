#ifndef BYTECODE_C
#define BYTECODE_C

#include <stdlib.h>
#include <stdbool.h>
#include "bytecode.h"
#include "../etc/strings.h"
#include "../etc/log.h"

String* get_string_in_G_Bytecode(const G_Bytecode* x, size_t i, const G_ubyte size_of_len_number)
{
    if (!x) return NULL;
    if (i + size_of_len_number -1 >= x->length) return NULL;

    String* str = init_String_ptr();
    if (!str) return NULL;

    size_t len;
    memcpy(&len, &x->bytecode[i], size_of_len_number);
    if (i + size_of_len_number + len -1 >= x->length) {
        clearstring_ptr(&str);
        return NULL;
    }

    i += size_of_len_number;


    stringconcat_char_w_len(str, (char*)x->bytecode + i, len);
    return str;
}

G_Bytecode init_G_Bytecode()
{
    G_Bytecode x = {0};
    x.size = 32;
    x.length = 0;
    x.bytecode = malloc(x.size);
    if (!x.bytecode) x.size = 0;
    return x;
}

G_Bytecode* init_G_Bytecode_ptr()
{
    G_Bytecode* x = malloc(sizeof(G_Bytecode));
    if (!x) return NULL;

    *x = init_G_Bytecode();
    if (!x->bytecode) {
        free(x);
        return NULL;
    }

    return x;
}

bool double_G_Bytecode_size(G_Bytecode* x)
{
    if (!x) return false;

    x->size *= 2;
    G_ubyte* tmp = realloc(x->bytecode, x->size * sizeof(G_ubyte));

    if (!tmp) {
        x->size /= 2;
        return false;
    }

    x->bytecode = tmp;
    return true;
}

G_Bytecode* add_G_Bytecode(G_Bytecode* x, const G_ubyte* data, const size_t data_length)
{
    if (!x || !data || !data_length) return NULL;

    while (x->size <= x->length + data_length) {
        if (!double_G_Bytecode_size(x))
            return NULL;
    }

    for (size_t i = 0; i < data_length; i++)
        x->bytecode[x->length++] = data[i];

    return x;
}

G_Bytecode* add_G_Bytecode_one_byte(G_Bytecode* x, const G_ubyte data)
{
    if (!x) return NULL;

    while (x->size <= x->length + 1) {
        if (!double_G_Bytecode_size(x))
            return NULL;
    }
    x->bytecode[x->length++] = data;

    return x;
}


G_Bytecode* add_G_Bytecode_w_byte_size(G_Bytecode* x, const void* data, const size_t length)
{
    return add_G_Bytecode(x, (G_ubyte*)data, length);
}



void destroy_G_Bytecode(G_Bytecode* x)
{
    if (!x) return;
    if (x->bytecode) {
        free(x->bytecode);
        x->bytecode = NULL;
    }
    x->length = 0;
    x->size   = 0;
}

void destroy_G_Bytecode_ptr(G_Bytecode** x)
{
    if (!x) return;
    if (*x) {
        destroy_G_Bytecode(*x);
        free(*x);
    }
    *x = NULL;
}


G_Bytecode* convert_String_to_Bytecode_String(const String* str)
{
    if (!str) return NULL;
    G_Bytecode* x = init_G_Bytecode_ptr();
    if (!x) return NULL;
    G_Bytecode* tmp = add_G_Bytecode(x, (G_ubyte*)str->content, str->length);
    if (!tmp) destroy_G_Bytecode_ptr(&x);
    return x;
}

G_Bytecode* add_G_Bytecode_String_no_size_embedded(G_Bytecode* x, const String* str)
{
    if (!x || !str) return NULL;
    //if (!add_G_Bytecode_w_byte_size(x, (G_ubyte*)&str->length, sizeof(size_t))) return NULL;
    if (!add_G_Bytecode(x, (G_ubyte*)str->content, str->length)) return NULL;
    return x;
}

#endif