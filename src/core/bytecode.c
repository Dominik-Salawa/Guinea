#ifndef BYTECODE_C
#define BYTECODE_C

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "bytecode.h"
#include "../etc/strings.h"
#include "../etc/stdlib/GUIN_stdio.h"
#include "../etc/log.h"

char* GUIN_GINSTR_Datatype_to_str(GINSTR_Datatype x)
{
    switch (x)
    {
        case GINSTRDATATYPE_NULL:       return "null";
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

GUIN_String* GUIN_get_string_in_Bytecode(const GUIN_Bytecode* x, size_t i, const GUIN_ubyte size_of_len_number)
{
    if (!x) return NULL;
    if (i + size_of_len_number -1 >= x->length) return NULL;

    GUIN_String* str = GUIN_init_String_ptr();
    if (!str) return NULL;

    size_t len;
    memcpy(&len, &x->bytecode[i], size_of_len_number);
    if (i + size_of_len_number + len -1 >= x->length) {
        GUIN_clearstring_ptr(&str);
        return NULL;
    }

    i += size_of_len_number;


    GUIN_stringconcat_char_w_len(str, (char*)x->bytecode + i, len);
    return str;
}

GUIN_Bytecode GUIN_init_Bytecode(void)
{
    GUIN_Bytecode x = {0};
    x.size = 32;
    x.length = 0;
    x.bytecode = malloc(x.size);
    if (!x.bytecode) x.size = 0;
    return x;
}

GUIN_Bytecode* GUIN_init_Bytecode_ptr(void)
{
    GUIN_Bytecode* x = malloc(sizeof(GUIN_Bytecode));
    if (!x) return NULL;

    *x = GUIN_init_Bytecode();
    if (!x->bytecode) {
        free(x);
        return NULL;
    }

    return x;
}

bool GUIN_double_Bytecode_size(GUIN_Bytecode* x)
{
    if (!x) return false;

    x->size *= 2;
    GUIN_ubyte* tmp = realloc(x->bytecode, x->size * sizeof(GUIN_ubyte));

    if (!tmp) {
        x->size /= 2;
        return false;
    }

    x->bytecode = tmp;
    return true;
}

bool GUIN_resize_Bytecode_size(GUIN_Bytecode* x, size_t newsize)
{
    if (!x) return false;
    if (newsize == x->size) return true;

    GUIN_ubyte* tmp = realloc(x->bytecode, newsize * sizeof(GUIN_ubyte));

    if (!tmp) {
        return false;
    }

    x->size = newsize;
    x->bytecode = tmp;
    return true;
}

GUIN_Bytecode* GUIN_add_Bytecode(GUIN_Bytecode* x, const GUIN_ubyte* data, const size_t data_length)
{
    if (!x || !data || !data_length) return NULL;

    {
        size_t newsize = x->size;
        while (newsize <= x->length + data_length) newsize *= 2;
        if (!GUIN_resize_Bytecode_size(x, newsize))
            return NULL;
    }

    for (size_t i = 0; i < data_length; i++)
        x->bytecode[x->length++] = data[i];

    return x;
}

GUIN_Bytecode* GUIN_add_Bytecode_one_byte(GUIN_Bytecode* x, const GUIN_ubyte data)
{
    if (!x) return NULL;

    {
        size_t newsize = x->size;
        while (newsize <= x->length + 1) newsize *= 2;
        if (!GUIN_resize_Bytecode_size(x, newsize))
            return NULL;
    }
    x->bytecode[x->length++] = data;

    return x;
}


GUIN_Bytecode* GUIN_add_Bytecode_w_byte_size(GUIN_Bytecode* x, const void* data, const size_t length)
{
    return GUIN_add_Bytecode(x, (GUIN_ubyte*)data, length);
}



void GUIN_destroy_Bytecode(GUIN_Bytecode* x)
{
    if (!x) return;
    if (x->bytecode) {
        free(x->bytecode);
        x->bytecode = NULL;
    }
    x->length = 0;
    x->size   = 0;
}

void GUIN_destroy_Bytecode_ptr(GUIN_Bytecode** x)
{
    if (!x) return;
    if (*x) {
        GUIN_destroy_Bytecode(*x);
        free(*x);
    }
    *x = NULL;
}


GUIN_Bytecode* GUIN_convert_String_to_Bytecode_String(const GUIN_String* str)
{
    if (!str) return NULL;
    GUIN_Bytecode* x = GUIN_init_Bytecode_ptr();
    if (!x) return NULL;
    GUIN_Bytecode* tmp = GUIN_add_Bytecode(x, (GUIN_ubyte*)str->content, str->length);
    if (!tmp) GUIN_destroy_Bytecode_ptr(&x);
    return x;
}

GUIN_Bytecode* GUIN_add_Bytecode_String(GUIN_Bytecode* x, const GUIN_String* str)
{
    if (!x || !str) return NULL;
    //if (!add_GUIN_Bytecode_w_byte_size(x, (GUIN_ubyte*)&str->length, sizeof(size_t))) return NULL;
    if (!GUIN_add_Bytecode(x, (GUIN_ubyte*)&str->length, sizeof(size_t))) return NULL;
    if (!GUIN_add_Bytecode(x, (GUIN_ubyte*)str->content, str->length))    return NULL;
    return x;
}


GUIN_Bytecode* GUIN_add_Bytecode_String_no_size_embedded(GUIN_Bytecode* x, const GUIN_String* str)
{
    if (!x || !str) return NULL;
    //if (!add_GUIN_Bytecode_w_byte_size(x, (GUIN_ubyte*)&str->length, sizeof(size_t))) return NULL;
    if (!GUIN_add_Bytecode(x, (GUIN_ubyte*)str->content, str->length)) return NULL;
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
bool GUIN_print_Bytecode_into_ASM(GUIN_Bytecode* x)
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

        GUIN_printf("%zu   ", i-6);

        switch (x->bytecode[i])
        {
            case GINSTR_DECLARE_GLOBAL: {
                ++i;
                GINSTR_Datatype globaldatatype = x->bytecode[i];

                GUIN_printf("DECLARE GLOBAL ");

                ++i;

                GUIN_String* name = GUIN_get_string_in_Bytecode(x, i, sizeof(size_t));
                if (!name) {
                    GUIN_printf("no name: \n");
                    valid = false;
                } else {
                    GUIN_printf("%s: ", name);
                }

                i += sizeof(size_t) + name->length; // varname len + int size
                GUIN_printf("%sc\n", GUIN_GINSTR_Datatype_to_str(globaldatatype));

                GUIN_clearstring_ptr(&name);
                break;
            }

            case GINSTR_DECLARE_LOCAL: {
                ++i;
                GINSTR_Datatype globaldatatype = x->bytecode[i];

                GUIN_printf("DECLARE LOCAL ");

                ++i;

                GUIN_LOCAL_SLOT_INT slotnum;
                memcpy(&slotnum, &x->bytecode[i], sizeof(GUIN_LOCAL_SLOT_INT));

                i += sizeof(GUIN_LOCAL_SLOT_INT); // varname len + int size
                GUIN_printf("%u: %sc\n", slotnum, GUIN_GINSTR_Datatype_to_str(globaldatatype));
                break;
            }

            case GINSTR_CLEAR_LOCAL: {
                ++i;

                GUIN_LOCAL_SLOT_INT slot;
                memcpy(&slot, &x->bytecode[i], sizeof(GUIN_LOCAL_SLOT_INT));
                GUIN_printf("CLEAR LOCAL: %d\n", slot);

                i += sizeof(GUIN_LOCAL_SLOT_INT); // varname len + int size
                break;
            }

            case GINSTR_LOAD_GLOBAL: {
                GUIN_printf("LOAD GLOBAL: ");
                ++i;

                GUIN_String* name = GUIN_get_string_in_Bytecode(x, i, sizeof(size_t));
                if (!name) {
                    GUIN_printf("no name\n");
                    valid = false;
                } else {
                    GUIN_printf("%s\n", name);
                }

                i += sizeof(size_t) + name->length; // varname len + int size
                GUIN_clearstring_ptr(&name);
                break;
            }

            case GINSTR_LOAD_LOCAL: {
                ++i;

                GUIN_LOCAL_SLOT_INT slot;
                memcpy(&slot, &x->bytecode[i], sizeof(GUIN_LOCAL_SLOT_INT));
                GUIN_printf("LOAD LOCAL: %d\n", slot);

                i += sizeof(GUIN_LOCAL_SLOT_INT); // varname len + int size
                break;
            }

            case GINSTR_LOAD_INDEX: {
                ++i;

                size_t slot;
                memcpy(&slot, &x->bytecode[i], sizeof(size_t));
                GUIN_printf("LOAD INDEX: %zu\n", slot);

                i += sizeof(size_t); // varname len + int size
                break;
            }

            case GINSTR_LOAD_FIELD: {
                GUIN_printf("LOAD FIELD ");
                ++i;

                GUIN_String* name = GUIN_get_string_in_Bytecode(x, i, sizeof(size_t));
                if (!name) {
                    GUIN_printf("no name\n");
                    valid = false;
                } else {
                    GUIN_printf("%s\n", name);
                }

                i += sizeof(size_t) + name->length; // varname len + int size
                GUIN_clearstring_ptr(&name);
                break;
            }

            case GINSTR_WRITE_LOAD: {
                GUIN_printf("WRITE LOAD\n");
                ++i;
                break;
            }

            case GINSTR_PUSH_LOAD: {
                GUIN_printf("PUSH LOAD\n");
                ++i;
                break;
            }

            case GINSTR_PUSH_IMMEDIATE: {
                GUIN_printf("PUSH ");
                ++i;
                GINSTR_Datatype immediatedatatype = x->bytecode[i];
                ++i;
                GUIN_printf("%sc ", GUIN_GINSTR_Datatype_to_str(immediatedatatype));
                switch (immediatedatatype)
                {
                    case GINSTRDATATYPE_STRING:
                    {
                        int64_t size;
                        memcpy(&size, &x->bytecode[i], 8);
                        i += sizeof(int64_t);
                        GUIN_String str = GUIN_init_String();
                        GUIN_stringconcat_char_w_len(&str, (char*)&x->bytecode[i], size);
                        GUIN_printf("\"%s\"", &str);
                        i += size;
                        GUIN_clearstring(&str);
                        break;
                    }

                    case GINSTRDATATYPE_NIL:
                    {
                        break; // dont need to do anything
                    }

                    case GINSTRDATATYPE_INT32:
                    {
                        GUIN_int32 val;
                        memcpy(&val, &x->bytecode[i], 4);
                        GUIN_printf("%d", val);
                        i += 4;
                        break;
                    }

                    case GINSTRDATATYPE_INT64:
                    {
                        GUIN_int64 val;
                        memcpy(&val, &x->bytecode[i], 8);
                        GUIN_printf("%ld", val);
                        i += 8;
                        break;
                    }

                    case GINSTRDATATYPE_NUMBER32:
                    {
                        GUIN_number32 val;
                        memcpy(&val, &x->bytecode[i], 4);
                        GUIN_printf("%f", val);
                        i += 4;
                        break;
                    }

                    case GINSTRDATATYPE_NUMBER64:
                    {
                        GUIN_number64 val;
                        memcpy(&val, &x->bytecode[i], 8);

                        GUIN_printf("%lf", val);
                        i += 8;
                        break;
                    }

                    case GINSTRDATATYPE_CHAR:
                    {
                        char ch = x->bytecode[i];
                        GUIN_printf("%c", ch);
                        ++i;
                        break;
                    }

                    case GINSTRDATATYPE_BOOL:
                    {
                        GUIN_printf("%b", x->bytecode[i]);
                        ++i;
                        break;
                    }

                    default:
                    {
                        GUIN_printf("[unknown type: %d:%zu]", immediatedatatype, i+1);
                        valid = false;
                        break;
                    }
                }
                putchar('\n');
                break;
            }



            case GINSTR_JMP: {
                GUIN_printf("JMP ");
                ++i;
                GUIN_int16 size;
                memcpy(&size, &x->bytecode[i], sizeof(GUIN_int16));
                i += sizeof(GUIN_int16);
                GUIN_printf("%d (%d)\n", size, i + (size - 6));
                break;
            }

            case GINSTR_JMPL: {
                GUIN_printf("JMPL ");
                ++i;
                GUIN_int64 size;
                memcpy(&size, &x->bytecode[i], sizeof(GUIN_int64));
                i += sizeof(GUIN_int64);
                GUIN_printf("%d (%d)\n", size, i + (size - 6));
                break;
            }

            case GINSTR_JNT: {
                GUIN_printf("JNT ");
                ++i;
                GUIN_int16 size;
                memcpy(&size, &x->bytecode[i], sizeof(GUIN_int16));
                i += sizeof(GUIN_int16);
                GUIN_printf("%d (%d)\n", size, i + (size - 6));
                break;
            }

            case GINSTR_JNTL: {
                GUIN_printf("JNTL ");
                ++i;
                GUIN_int64 size;
                memcpy(&size, &x->bytecode[i], sizeof(GUIN_int64));
                i += sizeof(GUIN_int64);
                GUIN_printf("%d (%d)\n", size, i + (size - 6));
                break;
            }

            case GINSTR_JIT: {
                GUIN_printf("JIT ");
                ++i;
                GUIN_int16 size;
                memcpy(&size, &x->bytecode[i], sizeof(GUIN_int16));
                i += sizeof(GUIN_int16);
                GUIN_printf("%d (%d)\n", size, i + (size - 6));
                break;
            }

            case GINSTR_JITL: {
                GUIN_printf("JITL ");
                ++i;
                GUIN_int64 size;
                memcpy(&size, &x->bytecode[i], sizeof(GUIN_int64));
                i += sizeof(GUIN_int64);
                GUIN_printf("%d (%d)\n", size, i + (size - 6));
                break;
            }



            case GINSTR_ADD: {
                GUIN_printf("ADD\n");
                ++i;
                break;
            }
            case GINSTR_SUB: {
                GUIN_printf("SUB\n");
                ++i;
                break;
            }
            case GINSTR_MUL: {
                GUIN_printf("MUL\n");
                ++i;
                break;
            }
            case GINSTR_DIV: {
                GUIN_printf("DIV\n");
                ++i;
                break;
            }
            case GINSTR_MOD: {
                GUIN_printf("MOD\n");
                ++i;
                break;
            }
            case GINSTR_POW: {
                GUIN_printf("POW\n");
                ++i;
                break;
            }
            case GINSTR_AND: {
                GUIN_printf("AND\n");
                ++i;
                break;
            }
            case GINSTR_OR: {
                GUIN_printf("OR\n");
                ++i;
                break;
            }
            case GINSTR_NOT: {
                GUIN_printf("NOT\n");
                ++i;
                break;
            }
            case GINSTR_NEG: {
                GUIN_printf("NEG\n");
                ++i;
                break;
            }
            case GINSTR_CALL: {
                GUIN_printf("CALL ");
                ++i;
                GUIN_ubyte size;
                memcpy(&size, &x->bytecode[i], sizeof(GUIN_ubyte));
                i += sizeof(GUIN_ubyte);
                GUIN_printf("%d\n", size);
                break;
            }
            case GINSTR_EQU: {
                GUIN_printf("EQU\n");
                ++i;
                break;
            }
            case GINSTR_NOT_EQU: {
                GUIN_printf("NOT-EQU\n");
                ++i;
                break;
            }
            case GINSTR_GT: {
                GUIN_printf("GT\n");
                ++i;
                break;
            }
            case GINSTR_LT: {
                GUIN_printf("LT\n");
                ++i;
                break;
            }
            case GINSTR_GT_EQU: {
                GUIN_printf("GT-EQU\n");
                ++i;
                break;
            }
            case GINSTR_LT_EQU: {
                GUIN_printf("LT-EQU\n");
                ++i;
                break;
            }
            default: {
                GUIN_printf("err %d\n", x->bytecode[i]);
                valid = false;
                ++i;
                break;
            }
        }
    }

    for (char j = 0; j < numberlen - numlen(i-6); ++j)
        putchar('0');
    GUIN_printf("%zu   END OF BYTECODE", i-6);
    return valid;
}




#include "compiler/ast.h"
GINSTR_Datatype GUIN_ASTDatatype_to_Bytecode_Datatype(GUIN_ASTDatatype x)
{
    switch (x)
    {
        case GUIN_ASTDATATYPE_NIL:           return GINSTRDATATYPE_NIL;
        case GUIN_ASTDATATYPE_STRING:        return GINSTRDATATYPE_STRING;
        case GUIN_ASTDATATYPE_INT:           return GINSTRDATATYPE_INT64;
        case GUIN_ASTDATATYPE_NUMBER:        return GINSTRDATATYPE_NUMBER64;
        case GUIN_ASTDATATYPE_BOOL:          return GINSTRDATATYPE_BOOL;
        case GUIN_ASTDATATYPE_CHAR:          return GINSTRDATATYPE_CHAR;
        case GUIN_ASTDATATYPE_FUNCTION:      return GINSTRDATATYPE_FUNCTION;
        case GUIN_ASTDATATYPE_DYNAMIC:       return GINSTRDATATYPE_DYNAMIC;
        default:
            printf("Couldnt find GUIN_BytecodeDatatype equivalent of ASTDatatype %d!\n", x);
            exit(1);
    }
}

#endif
