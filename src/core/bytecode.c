#ifndef guin_BYTECODE_c
#define guin_BYTECODE_c

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "bytecode.h"
#include "../etc/strings.h"
#include "../etc/stdlib/stdio.h"
#include "../etc/log.h"
#include "../../include/declarations.h"

char* guin_GINSTR_Datatype_to_string(GINSTR_Datatype x)
{
    switch (x)
    {
        case GINSTRDATATYPE_NULL:       return "null";
        case GINSTRDATATYPE_VOID:       return "void";
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
    }
	return "(null)";
}

guin_String* guin_get_string_in_Bytecode(const guin_Bytecode* x, size_t i, const guin_ubyte size_of_len_number)
{
    if (!x) return NULL;
    if (i + size_of_len_number -1 >= x->length) return NULL;

    guin_String* str = guin_init_String_ptr();
    if (!str) return NULL;

    size_t len;
    memcpy(&len, &x->bytecode[i], size_of_len_number);
    if (i + size_of_len_number + len -1 >= x->length) {
        guin_clearstring_ptr(&str);
        return NULL;
    }

    i += size_of_len_number;

    guin_stringconcat_char_w_len(str, (char*)x->bytecode + i, len);
    return str;
}

guin_Bytecode guin_init_Bytecode(void)
{
    guin_Bytecode x = {0};
    x.size = 32;
    x.length = 0;
    x.bytecode = guin_malloc(x.size);
    if (!x.bytecode) x.size = 0;
    return x;
}

guin_Bytecode* guin_init_Bytecode_ptr(void)
{
    guin_Bytecode* x = guin_malloc(sizeof(guin_Bytecode));
    if (!x) return NULL;

    *x = guin_init_Bytecode();
    if (!x->bytecode) {
        guin_mfree(x);
        return NULL;
    }

    return x;
}

bool guin_double_Bytecode_size(guin_Bytecode* x)
{
    if (!x) return false;

    x->size *= 2;
    guin_ubyte* tmp = guin_remalloc(x->bytecode, x->size * sizeof(guin_ubyte));

    if (!tmp) {
        x->size /= 2;
        return false;
    }

    x->bytecode = tmp;
    return true;
}

bool guin_resize_Bytecode_size(guin_Bytecode* x, size_t newsize)
{
    if (!x) return false;
    if (newsize == x->size) return true;

    guin_ubyte* tmp = guin_remalloc(x->bytecode, newsize * sizeof(guin_ubyte));

    if (!tmp) {
        return false;
    }

    x->size = newsize;
    x->bytecode = tmp;
    return true;
}

guin_Bytecode* guin_add_Bytecode(guin_Bytecode* x, const guin_ubyte* data, const size_t data_length)
{
    if (!x || !data || !data_length) return NULL;

    {
        size_t newsize = x->size;
        while (newsize <= x->length + data_length) newsize *= 2;
        if (!guin_resize_Bytecode_size(x, newsize))
            return NULL;
    }


    for (size_t i = 0; i < data_length; i++)
        x->bytecode[x->length++] = data[i];

    return x;
}

guin_Bytecode* guin_add_Bytecode_one_byte(guin_Bytecode* x, const guin_ubyte data)
{
    if (!x) return NULL;

    {
        size_t newsize = x->size;
        while (newsize <= x->length + 1) newsize *= 2;
        if (!guin_resize_Bytecode_size(x, newsize))
            return NULL;
    }
    x->bytecode[x->length++] = data;
    return x;
}


guin_Bytecode* guin_add_Bytecode_w_byte_size(guin_Bytecode* x, const void* data, const size_t length)
{
    return guin_add_Bytecode(x, (guin_ubyte*)data, length);
}



void guin_destroy_Bytecode(guin_Bytecode* x)
{
    if (!x) return;
    if (x->bytecode) {
        guin_mfree(x->bytecode);
        x->bytecode = NULL;
    }
    x->length = 0;
    x->size   = 0;
}

void guin_destroy_Bytecode_ptr(guin_Bytecode** x)
{
    if (!x) return;
    if (*x) {
        guin_destroy_Bytecode(*x);
        guin_mfree(*x);
    }
    *x = NULL;
}


guin_Bytecode* guin_convert_String_to_Bytecode_String(const guin_String* str)
{
    if (!str) return NULL;
    guin_Bytecode* x = guin_init_Bytecode_ptr();
    if (!x) return NULL;
    guin_Bytecode* tmp = guin_add_Bytecode(x, (guin_ubyte*)str->content, str->length);
    if (!tmp) guin_destroy_Bytecode_ptr(&x);
    return x;
}

guin_Bytecode* guin_add_Bytecode_String(guin_Bytecode* x, const guin_String* str)
{
    if (!x || !str) return NULL;
    //if (!add_guin_Bytecode_w_byte_size(x, (guin_ubyte*)&str->length, sizeof(size_t))) return NULL;
    if (!guin_add_Bytecode(x, (guin_ubyte*)&str->length, sizeof(size_t))) return NULL;
	if (sizeof(size_t) < 8) {
		for (int i = 0; i < (int)(8-sizeof(size_t)); ++i)
			if (!guin_add_Bytecode_one_byte(x, 0)) return NULL;
	}
    if (!guin_add_Bytecode(x, (guin_ubyte*)str->content, str->length)) return NULL;
    return x;
}


bool guin_safe_memcpy_Bytecode(void* destination, guin_Bytecode* src, guin_ubyte* start, size_t length)
{
    guin_ubyte* ptr = (guin_ubyte*)destination;

    if (guin_out_of_bounds_from_Bytecode(src, (start + length - 1)))
        return false;

    for (size_t i = 0; i < length; ++i)
        *ptr++ = *(start + i);

    return true;
}


guin_Bytecode* guin_add_Bytecode_String_no_size_embedded(guin_Bytecode* x, const guin_String* str)
{
    if (!x || !str) return NULL;
    //if (!add_guin_Bytecode_w_byte_size(x, (guin_ubyte*)&str->length, sizeof(size_t))) return NULL;
    if (!guin_add_Bytecode(x, (guin_ubyte*)str->content, str->length)) return NULL;
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
bool guin_print_Bytecode_into_ASM(guin_Bytecode* x)
{
    bool valid = true;

    int numberlen = 8;
    if (numlen(x->length) > numberlen) {
        numberlen = numlen(x->length);
    }
    size_t i = 0;
    for (; i < x->length;) {
        for (char j = 0; j < numberlen - numlen(i); ++j)
            putchar('0');

        guin_printf("%zu   ", i);

        switch (x->bytecode[i])
        {
			case GINSTR_POP:
				puts("POP\n");
				++i;
				break;
				
            case GINSTR_RET:
                puts("RET\n");
                ++i;
                break;

            case GINSTR_DECLARE_GLOBAL: {
                ++i;
                GINSTR_Datatype globaldatatype = x->bytecode[i];

                guin_printf("DECLARE GLOBAL ");

                ++i;

                guin_String* name = guin_get_string_in_Bytecode(x, i, sizeof(size_t));
                if (!name) {
                    guin_printf("no name: \n");
                    valid = false;
                } else {
                    guin_printf("%s: ", name);
                }

                i += sizeof(size_t) + name->length; // varname len + int size
                guin_printf("%sc\n", guin_GINSTR_Datatype_to_string(globaldatatype));

                guin_clearstring_ptr(&name);
                break;
            }

            case GINSTR_DECLARE_LOCAL: {
                ++i;
                GINSTR_Datatype globaldatatype = x->bytecode[i];

                guin_printf("DECLARE LOCAL ");

                ++i;

                guin_LOCAL_SLOT_INT slotnum;
                memcpy(&slotnum, &x->bytecode[i], sizeof(guin_LOCAL_SLOT_INT));

                i += sizeof(guin_LOCAL_SLOT_INT); // varname len + int size
                guin_printf("%u: %sc\n", slotnum, guin_GINSTR_Datatype_to_string(globaldatatype));
                break;
            }

            case GINSTR_CLEAR_LOCAL: {
                ++i;

                guin_LOCAL_SLOT_INT slot;
                memcpy(&slot, &x->bytecode[i], sizeof(guin_LOCAL_SLOT_INT));
                guin_printf("CLEAR LOCAL: %d\n", slot);

                i += sizeof(guin_LOCAL_SLOT_INT); // varname len + int size
                break;
            }

            case GINSTR_LOAD_GLOBAL: {
                guin_printf("LOAD GLOBAL: ");
                ++i;

                guin_String* name = guin_get_string_in_Bytecode(x, i, sizeof(guin_uint64));
                if (!name) {
                    guin_printf("no name\n");
                    valid = false;
                } else {
                    guin_printf("%s\n", name);
                }

                i += sizeof(size_t) + name->length; // varname len + int size
                guin_clearstring_ptr(&name);
                break;
            }

            case GINSTR_LOAD_LOCAL: {
                ++i;

                guin_LOCAL_SLOT_INT slot;
                memcpy(&slot, &x->bytecode[i], sizeof(guin_LOCAL_SLOT_INT));
                guin_printf("LOAD LOCAL: %d\n", slot);

                i += sizeof(guin_LOCAL_SLOT_INT); // varname len + int size
                break;
            }

            case GINSTR_LOAD_INDEX: {
                ++i;

                size_t slot;
                memcpy(&slot, &x->bytecode[i], sizeof(size_t));
                guin_printf("LOAD INDEX: %zu\n", slot);

                i += sizeof(size_t); // varname len + int size
                break;
            }

            case GINSTR_LOAD_FIELD: {
                guin_printf("LOAD FIELD ");
                ++i;

                guin_String* name = guin_get_string_in_Bytecode(x, i, sizeof(size_t));
                if (!name) {
                    guin_printf("no name\n");
                    valid = false;
                } else {
                    guin_printf("%s\n", name);
                }

                i += sizeof(size_t) + name->length; // varname len + int size
                guin_clearstring_ptr(&name);
                break;
            }

            case GINSTR_WRITE_LOAD: {
                guin_printf("WRITE LOAD\n");
                ++i;
                break;
            }

            case GINSTR_PUSH_LOAD: {
                guin_printf("PUSH LOAD\n");
                ++i;
                break;
            }

            case GINSTR_PUSH_IMMEDIATE: {
                guin_printf("PUSH ");
                ++i;
                GINSTR_Datatype immediatedatatype = x->bytecode[i];
                ++i;
                guin_printf("%sc ", guin_GINSTR_Datatype_to_string(immediatedatatype));
                switch (immediatedatatype)
                {
                    case GINSTRDATATYPE_STRING:
                    {
                        guin_int64 size;
                        memcpy(&size, &x->bytecode[i], 8);
                        i += sizeof(guin_int64);
                        guin_String str = guin_init_String();
                        guin_stringconcat_char_w_len(&str, (char*)&x->bytecode[i], size);
                        guin_printf("\"%s\"", &str);
                        i += size;
                        guin_clearstring(&str);
                        break;
                    }

                    case GINSTRDATATYPE_NIL:
                    {
                        break; // dont need to do anything
                    }

                    case GINSTRDATATYPE_INT32:
                    {
                        guin_int32 val;
                        memcpy(&val, &x->bytecode[i], 4);
                        guin_printf("%d", val);
                        i += 4;
                        break;
                    }

                    case GINSTRDATATYPE_INT64:
                    {
                        guin_int64 val;
                        memcpy(&val, &x->bytecode[i], 8);
                        guin_printf("%ld", val);
                        i += 8;
                        break;
                    }

                    case GINSTRDATATYPE_NUMBER32:
                    {
                        guin_number32 val;
                        memcpy(&val, &x->bytecode[i], 4);
                        guin_printf("%f", val);
                        i += 4;
                        break;
                    }

                    case GINSTRDATATYPE_NUMBER64:
                    {
                        guin_number64 val;
                        memcpy(&val, &x->bytecode[i], 8);

                        guin_printf("%lf", val);
                        i += 8;
                        break;
                    }

                    case GINSTRDATATYPE_CHAR:
                    {
                        char ch = x->bytecode[i];
                        guin_printf("%c", ch);
                        ++i;
                        break;
                    }

                    case GINSTRDATATYPE_BOOL:
                    {
                        guin_printf("%b", x->bytecode[i]);
                        ++i;
                        break;
                    }

                    case GINSTRDATATYPE_FUNCTION:
                    {
                        printf("ret: %s | ", guin_GINSTR_Datatype_to_string(x->bytecode[i]));
                        ++i;
                        guin_uint64 val;
                        memcpy(&val, &x->bytecode[i], 8);
                        i += sizeof(val);
                        guin_printf("size: %lu (final-opcode:%lu)", val, i + (val) - 1);
                        break;
                    }

                    default:
                    {
                        guin_printf("[unknown type: %d:%zu]", immediatedatatype, i);
                        valid = false;
                        break;
                    }
                }
                putchar('\n');
                break;
            }



            case GINSTR_JMP: {
                guin_printf("JMP ");
                ++i;
                guin_int16 size;
                memcpy(&size, &x->bytecode[i], sizeof(guin_int16));
                i += sizeof(guin_int16);
                guin_printf("%d (%d)\n", size, i + (size));
                break;
            }

            case GINSTR_JMPL: {
                guin_printf("JMPL ");
                ++i;
                guin_int64 size;
                memcpy(&size, &x->bytecode[i], sizeof(guin_int64));
                i += sizeof(guin_int64);
                guin_printf("%d (%d)\n", size, i + (size));
                break;
            }

            case GINSTR_JNT: {
                guin_printf("JNT ");
                ++i;
                guin_int16 size;
                memcpy(&size, &x->bytecode[i], sizeof(guin_int16));
                i += sizeof(guin_int16);
                guin_printf("%d (%d)\n", size, i + (size));
                break;
            }

            case GINSTR_JNTL: {
                guin_printf("JNTL ");
                ++i;
                guin_int64 size;
                memcpy(&size, &x->bytecode[i], sizeof(guin_int64));
                i += sizeof(guin_int64);
                guin_printf("%d (%d)\n", size, i + (size));
                break;
            }

            case GINSTR_JIT: {
                guin_printf("JIT ");
                ++i;
                guin_int16 size;
                memcpy(&size, &x->bytecode[i], sizeof(guin_int16));
                i += sizeof(guin_int16);
                guin_printf("%d (%d)\n", size, i + (size));
                break;
            }

            case GINSTR_JITL: {
                guin_printf("JITL ");
                ++i;
                guin_int64 size;
                memcpy(&size, &x->bytecode[i], sizeof(guin_int64));
                i += sizeof(guin_int64);
                guin_printf("%d (%d)\n", size, i + (size));
                break;
            }



            case GINSTR_ADD: {
                guin_printf("ADD\n");
                ++i;
                break;
            }
            case GINSTR_SUB: {
                guin_printf("SUB\n");
                ++i;
                break;
            }
            case GINSTR_MUL: {
                guin_printf("MUL\n");
                ++i;
                break;
            }
            case GINSTR_DIV: {
                guin_printf("DIV\n");
                ++i;
                break;
            }
            case GINSTR_MOD: {
                guin_printf("MOD\n");
                ++i;
                break;
            }
            case GINSTR_POW: {
                guin_printf("POW\n");
                ++i;
                break;
            }
            case GINSTR_AND: {
                guin_printf("AND\n");
                ++i;
                break;
            }
            case GINSTR_OR: {
                guin_printf("OR\n");
                ++i;
                break;
            }
            case GINSTR_NOT: {
                guin_printf("NOT\n");
                ++i;
                break;
            }
            case GINSTR_NEG: {
                guin_printf("NEG\n");
                ++i;
                break;
            }
            case GINSTR_CALL: {
                guin_printf("CALL ");
                ++i;
                guin_ubyte size;
                memcpy(&size, &x->bytecode[i], sizeof(guin_ubyte));
                i += sizeof(guin_ubyte);
                guin_printf("%d\n", size);
                break;
            }
            case GINSTR_EQU: {
                guin_printf("EQU\n");
                ++i;
                break;
            }
            case GINSTR_NOT_EQU: {
                guin_printf("NOT-EQU\n");
                ++i;
                break;
            }
            case GINSTR_GT: {
                guin_printf("GT\n");
                ++i;
                break;
            }
            case GINSTR_LT: {
                guin_printf("LT\n");
                ++i;
                break;
            }
            case GINSTR_GT_EQU: {
                guin_printf("GT-EQU\n");
                ++i;
                break;
            }
            case GINSTR_LT_EQU: {
                guin_printf("LT-EQU\n");
                ++i;
                break;
            }
            default: {
                guin_printf("err %d\n", x->bytecode[i]);
                valid = false;
                ++i;
                break;
            }
        }
    }
    return valid;
}




#include "compiler/ast.h"
GINSTR_Datatype guin_ASTDatatype_to_Bytecode_Datatype(guin_ASTDatatype x)
{
    switch (x)
    {
        case guin_ASTDATATYPE_NIL:           return GINSTRDATATYPE_NIL;
        case guin_ASTDATATYPE_STRING:        return GINSTRDATATYPE_STRING;
        case guin_ASTDATATYPE_INT:           return GINSTRDATATYPE_INT64;
        case guin_ASTDATATYPE_NUMBER:        return GINSTRDATATYPE_NUMBER64;
        case guin_ASTDATATYPE_BOOL:          return GINSTRDATATYPE_BOOL;
        case guin_ASTDATATYPE_CHAR:          return GINSTRDATATYPE_CHAR;
        case guin_ASTDATATYPE_FUNCTION:      return GINSTRDATATYPE_FUNCTION;
        case guin_ASTDATATYPE_DYNAMIC:       return GINSTRDATATYPE_DYNAMIC;
		case guin_ASTDATATYPE_ERR:			 return GINSTRDATATYPE_NULL;
		case guin_ASTDATATYPE_VOID:			 return GINSTRDATATYPE_VOID;
    }
	return GINSTRDATATYPE_NULL;
}

#endif
