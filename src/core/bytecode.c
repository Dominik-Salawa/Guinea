#ifndef BYTECODE_C
#define BYTECODE_C

#include <stdlib.h>
#include <stdbool.h>
#include "bytecode.h"
#include "../etc/strings.h"
#include "../etc/stdlib/G_stdio.h"
#include "../etc/log.h"

char* G_Bytecode_Datatype_to_str(GINSTR_Datatype x)
{
    switch (x)
    {
        case GINSTRDATATYPE_NIL:        return "nil";
        case GINSTRDATATYPE_STRING:     return "string";
        case GINSTRDATATYPE_INT32:      return "int32";
        case GINSTRDATATYPE_INT64:      return "int64";
        case GINSTRDATATYPE_NUMBER32:   return "num32";
        case GINSTRDATATYPE_NUMBER64:   return "num64";
        case GINSTRDATATYPE_BOOL:       return "bool";
        case GINSTRDATATYPE_CHAR:       return "char";
        case GINSTRDATATYPE_FUNCTION:   return "function";
        case GINSTRDATATYPE_DYNAMIC:    return "dynamic";
        default: return "(null)";
    }
}

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

bool resize_G_Bytecode_size(G_Bytecode* x, size_t newsize)
{
    if (!x) return false;
    if (newsize == x->size) return true;

    G_ubyte* tmp = realloc(x->bytecode, newsize * sizeof(G_ubyte));

    if (!tmp) {
        return false;
    }

    x->size = newsize;
    x->bytecode = tmp;
    return true;
}

G_Bytecode* add_G_Bytecode(G_Bytecode* x, const G_ubyte* data, const size_t data_length)
{
    if (!x || !data || !data_length) return NULL;

    {
        size_t newsize = x->size;
        while (newsize <= x->length + data_length) newsize *= 2;
        if (!resize_G_Bytecode_size(x, newsize))
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




static int numlen(size_t number)
{
    int i = 1;
    while (number / 10 != 0) {
        number /= 10;
        ++i;
    }
    return i;
}

#include <string.h>
bool print_G_Bytecode_into_G_ASM(G_Bytecode* x)
{
    bool valid = true;

    int numberlen = 8;
    if (numlen(x->length) > numberlen) {
        numberlen = numlen(x->length);
    }
    size_t i = 7;
    for (; i < x->length;) {
        for (char j = 0; j < numberlen - numlen(i-6); ++j)
            putchar('0');

        G_printf("%zu   ", i-6);

        switch (x->bytecode[i])
        {
            case GINSTR_DECLARE_GLOBAL: {
                ++i;
                GINSTR_Datatype globaldatatype = x->bytecode[i];

                G_printf("DECLARE GLOBAL ");

                ++i;

                String* name = get_string_in_G_Bytecode(x, i, sizeof(size_t));
                if (!name) {
                    G_printf("no name: \n");
                    valid = false;
                } else {
                    G_printf("%s: ", *name);
                }

                i += sizeof(size_t) + name->length; // varname len + int size
                G_printf("%sc\n", G_Bytecode_Datatype_to_str(globaldatatype));

                clearstring_ptr(&name);
                break;
            }

            case GINSTR_DECLARE_LOCAL: {
                ++i;
                GINSTR_Datatype globaldatatype = x->bytecode[i];

                G_printf("DECLARE LOCAL ");

                ++i;

                G_LOCAL_SLOT_INT slotnum;
                memcpy(&slotnum, &x->bytecode[i], sizeof(G_LOCAL_SLOT_INT));

                i += sizeof(G_LOCAL_SLOT_INT); // varname len + int size
                G_printf("%u: %sc\n", slotnum, G_Bytecode_Datatype_to_str(globaldatatype));
                break;
            }

            case GINSTR_CLEAR_LOCAL: {
                ++i;

                G_LOCAL_SLOT_INT slot;
                memcpy(&slot, &x->bytecode[i], sizeof(G_LOCAL_SLOT_INT));
                G_printf("CLEAR LOCAL: %d\n", slot);

                i += sizeof(G_LOCAL_SLOT_INT); // varname len + int size
                break;
            }

            case GINSTR_LOAD_GLOBAL: {
                G_printf("LOAD GLOBAL: ");
                ++i;

                String* name = get_string_in_G_Bytecode(x, i, sizeof(size_t));
                if (!name) {
                    G_printf("no name\n");
                    valid = false;
                } else {
                    G_printf("%s\n", *name);
                }

                i += sizeof(size_t) + name->length; // varname len + int size
                clearstring_ptr(&name);
                break;
            }

            case GINSTR_LOAD_LOCAL: {
                ++i;

                G_LOCAL_SLOT_INT slot;
                memcpy(&slot, &x->bytecode[i], sizeof(G_LOCAL_SLOT_INT));
                G_printf("LOAD LOCAL: %d\n", slot);

                i += sizeof(G_LOCAL_SLOT_INT); // varname len + int size
                break;
            }

            case GINSTR_LOAD_INDEX: {
                ++i;

                size_t slot;
                memcpy(&slot, &x->bytecode[i], sizeof(size_t));
                G_printf("LOAD INDEX: %zu\n", slot);

                i += sizeof(size_t); // varname len + int size
                break;
            }

            case GINSTR_LOAD_FIELD: {
                G_printf("LOAD FIELD ");
                ++i;

                String* name = get_string_in_G_Bytecode(x, i, sizeof(size_t));
                if (!name) {
                    G_printf("no name\n");
                    valid = false;
                } else {
                    G_printf("%s\n", *name);
                }

                i += sizeof(size_t) + name->length; // varname len + int size
                clearstring_ptr(&name);
                break;
            }

            case GINSTR_WRITE_LOAD: {
                G_printf("WRITE LOAD\n");
                ++i;
                break;
            }

            case GINSTR_PUSH_LOAD: {
                G_printf("PUSH LOAD\n");
                ++i;
                break;
            }

            case GINSTR_PUSH_IMMEDIATE: {
                G_printf("PUSH ");
                ++i;
                GINSTR_Datatype immediatedatatype = x->bytecode[i];
                ++i;
                G_printf("%sc ", G_Bytecode_Datatype_to_str(immediatedatatype));
                switch (immediatedatatype)
                {
                    case GINSTRDATATYPE_STRING:
                    {
                        int64_t size;
                        memcpy(&size, &x->bytecode[i], 8);
                        i += sizeof(int64_t);
                        String str = init_String();
                        stringconcat_char_w_len(&str, (char*)&x->bytecode[i], size);
                        G_printf("\"%s\"", str);
                        i += size;
                        clearstring(&str);
                        break;
                    }

                    case GINSTRDATATYPE_NIL:
                    {
                        break; // dont need to do anything
                    }

                    case GINSTRDATATYPE_INT32:
                    {
                        G_int32 val;
                        memcpy(&val, &x->bytecode[i], 4);
                        G_printf("%d", val);
                        i += 4;
                        break;
                    }

                    case GINSTRDATATYPE_INT64:
                    {
                        G_int64 val;
                        memcpy(&val, &x->bytecode[i], 8);
                        G_printf("%ld", val);
                        i += 8;
                        break;
                    }

                    case GINSTRDATATYPE_NUMBER32:
                    {
                        G_number32 val;
                        memcpy(&val, &x->bytecode[i], 4);
                        G_printf("%f", val);
                        i += 4;
                        break;
                    }

                    case GINSTRDATATYPE_NUMBER64:
                    {
                        G_number64 val;
                        memcpy(&val, &x->bytecode[i], 8);

                        G_printf("%lf", val);
                        i += 8;
                        break;
                    }

                    case GINSTRDATATYPE_CHAR:
                    {
                        char ch = x->bytecode[i];
                        G_printf("%c", ch);
                        ++i;
                        break;
                    }

                    case GINSTRDATATYPE_BOOL:
                    {
                        printf((x->bytecode[i] != 0)? "true" : "false");
                        ++i;
                        break;
                    }

                    default:
                    {
                        G_printf("[unknown type: %d:%zu]", immediatedatatype, i+1);
                        valid = false;
                        break;
                    }
                }
                putchar('\n');
                break;
            }
           
            

            case GINSTR_JMP: {
                G_printf("JMP ");
                ++i;
                G_int16 size;
                memcpy(&size, &x->bytecode[i], sizeof(G_int16));
                i += sizeof(G_int16);
                G_printf("%d (%d)\n", size, i + (size - 6));
                break;
            }

            case GINSTR_JMPL: {
                G_printf("JMPL ");
                ++i;
                G_int64 size;
                memcpy(&size, &x->bytecode[i], sizeof(G_int64));
                i += sizeof(G_int64);
                G_printf("%d (%d)\n", size, i + (size - 6));
                break;
            }

            case GINSTR_JNT: {
                G_printf("JNT ");
                ++i;
                G_int16 size;
                memcpy(&size, &x->bytecode[i], sizeof(G_int16));
                i += sizeof(G_int16);
                G_printf("%d (%d)\n", size, i + (size - 6));
                break;
            }

            case GINSTR_JNTL: {
                G_printf("JNTL ");
                ++i;
                G_int64 size;
                memcpy(&size, &x->bytecode[i], sizeof(G_int64));
                i += sizeof(G_int64);
                G_printf("%d (%d)\n", size, i + (size - 6));
                break;
            }

            case GINSTR_JIT: {
                G_printf("JIT ");
                ++i;
                G_int16 size;
                memcpy(&size, &x->bytecode[i], sizeof(G_int16));
                i += sizeof(G_int16);
                G_printf("%d (%d)\n", size, i + (size - 6));
                break;
            }

            case GINSTR_JITL: {
                G_printf("JITL ");
                ++i;
                G_int64 size;
                memcpy(&size, &x->bytecode[i], sizeof(G_int64));
                i += sizeof(G_int64);
                G_printf("%d (%d)\n", size, i + (size - 6));
                break;
            }



            case GINSTR_ADD: {
                G_printf("ADD\n");
                ++i;
                break;
            }
            case GINSTR_SUB: {
                G_printf("SUB\n");
                ++i;
                break;
            }
            case GINSTR_MUL: {
                G_printf("MUL\n");
                ++i;
                break;
            }
            case GINSTR_DIV: {
                G_printf("DIV\n");
                ++i;
                break;
            }
            case GINSTR_MOD: {
                G_printf("MOD\n");
                ++i;
                break;
            }
            case GINSTR_POW: {
                G_printf("POW\n");
                ++i;
                break;
            }
            case GINSTR_AND: {
                G_printf("AND\n");
                ++i;
                break;
            }
            case GINSTR_OR: {
                G_printf("OR\n");
                ++i;
                break;
            }
            case GINSTR_NOT: {
                G_printf("NOT\n");
                ++i;
                break;
            }
            case GINSTR_NEG: {
                G_printf("NEG\n");
                ++i;
                break;
            }
            case GINSTR_CALL: {
                G_printf("CALL\n");
                ++i;
                break;
            }
            case GINSTR_EQU: {
                G_printf("EQU\n");
                ++i;
                break;
            }
            case GINSTR_NOT_EQU: {
                G_printf("NOT-EQU\n");
                ++i;
                break;
            }
            case GINSTR_GT: {
                G_printf("GT\n");
                ++i;
                break;
            }
            case GINSTR_LT: {
                G_printf("LT\n");
                ++i;
                break;
            }
            case GINSTR_GT_EQU: {
                G_printf("GT-EQU\n");
                ++i;
                break;
            }
            case GINSTR_LT_EQU: {
                G_printf("LT-EQU\n");
                ++i;
                break;
            }
            default: {
                G_printf("err %d\n", x->bytecode[i]);
                valid = false;
                ++i;
                break;
            }
        }
    }

    for (char j = 0; j < numberlen - numlen(i-6); ++j)
        putchar('0');
    G_printf("%zu   END OF BYTECODE", i-6);
    return valid;
}




#include "compiler/ast.h"
GINSTR_Datatype ASTDatatype_to_G_Bytecode_Datatype(ASTDatatype x)
{
    switch (x)
    {
        case ASTDATATYPE_NIL:           return GINSTRDATATYPE_NIL;
        case ASTDATATYPE_STRING:        return GINSTRDATATYPE_STRING;
        case ASTDATATYPE_INT:           return GINSTRDATATYPE_INT64;
        case ASTDATATYPE_NUMBER:        return GINSTRDATATYPE_NUMBER64;
        case ASTDATATYPE_BOOL:          return GINSTRDATATYPE_BOOL;
        case ASTDATATYPE_CHAR:          return GINSTRDATATYPE_CHAR;
        case ASTDATATYPE_FUNCTION:      return GINSTRDATATYPE_FUNCTION;
        case ASTDATATYPE_DYNAMIC:       return GINSTRDATATYPE_DYNAMIC;
        default:
            printf("Couldnt find G_BytecodeDatatype equivalent of ASTDatatype %d!\n", x);
            exit(1);
    }
}

#endif