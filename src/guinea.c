#ifndef guin_c
#define guin_c

// THIS FILES ENTIRE PURPOSE IS TO QUICKLY LINK AND INCLUDE ALL THE .c FILES FOR GUINEA
// /include/guinea.h DOES NOT SERVE THE SAME PURPOSE AND IT IS JUST FOR EMBEDDING API
// guinea.h is located in /include/

#include "../include/guinea.h"
#include "guinea.h"

#include "etc/stdlib/stdio.h"

#include "declarations.c"

#include "etc/log.c"
#include "etc/strings.c"
#include "etc/string-manipulation.c"
#include "etc/stdlib/stdio.c"

#include "core/compiler.c"
#include "core/interpreter.c"

#include "core/ver.h"

guin_API_Value guin_API_nil(void* _)                            { return (guin_API_Value){.type=guin_API_DT_NIL};           }
guin_API_Value guin_API_int16(guin_int16 data)                  { return (guin_API_Value){.type=guin_API_DT_I16,.i16=data}; }
guin_API_Value guin_API_int32(guin_int32 data)                  { return (guin_API_Value){.type=guin_API_DT_I32,.i32=data}; }
guin_API_Value guin_API_int64(guin_int64 data)                  { return (guin_API_Value){.type=guin_API_DT_I64,.i64=data}; }
guin_API_Value guin_API_number32(guin_number32 data)            { return (guin_API_Value){.type=guin_API_DT_N32,.n32=data}; }
guin_API_Value guin_API_number64(guin_number64 data)            { return (guin_API_Value){.type=guin_API_DT_N64,.n64=data}; }
guin_API_Value guin_API_char(char data)                         { return (guin_API_Value){.type=guin_API_DT_CHAR,.ch=data}; }
guin_API_Value guin_API_bool(bool data)                         { return (guin_API_Value){.type=guin_API_DT_BOOL,.bl=data}; }
guin_API_Value guin_API_string(const char* data)                { return (guin_API_Value){.type=guin_API_DT_STR,.str=data}; }

guin_ValueHeader guin_API_Value_to_ValueHeader(guin_API_Value x)
{
    switch (x.type)
    {
        case guin_API_DT_NIL:  return (guin_ValueHeader){.header_type=GINSTRDATATYPE_NIL,.current_value_type=GINSTRDATATYPE_NIL};
        case guin_API_DT_I16:  return (guin_ValueHeader){.i32=x.i16,.header_type=GINSTRDATATYPE_INT32,.current_value_type=GINSTRDATATYPE_INT32};
        case guin_API_DT_I32:  return (guin_ValueHeader){.i32=x.i32,.header_type=GINSTRDATATYPE_INT32,.current_value_type=GINSTRDATATYPE_INT32};
        case guin_API_DT_I64:  return (guin_ValueHeader){.i64=x.i64,.header_type=GINSTRDATATYPE_INT64,.current_value_type=GINSTRDATATYPE_INT64};
        case guin_API_DT_N32:  return (guin_ValueHeader){.n32=x.n32,.header_type=GINSTRDATATYPE_NUMBER32,.current_value_type=GINSTRDATATYPE_NUMBER32};
        case guin_API_DT_N64:  return (guin_ValueHeader){.n64=x.n64,.header_type=GINSTRDATATYPE_NUMBER64,.current_value_type=GINSTRDATATYPE_NUMBER64};
        case guin_API_DT_CHAR: return (guin_ValueHeader){.ch=x.ch,.header_type=GINSTRDATATYPE_CHAR,.current_value_type=GINSTRDATATYPE_CHAR};
        case guin_API_DT_BOOL: return (guin_ValueHeader){.bl=x.bl,.header_type=GINSTRDATATYPE_BOOL,.current_value_type=GINSTRDATATYPE_BOOL};
        case guin_API_DT_STR: {
            guin_String* s = guin_init_String_ptr();
            guin_stringconcat_charptr(s, x.str);
            return (guin_ValueHeader){.str=s,.header_type=GINSTRDATATYPE_STRING,.current_value_type=GINSTRDATATYPE_STRING};
        }
        default: return (guin_ValueHeader){0};
    }
}

guin_VM* guin_API_open_VM(void)
{
    return guin_init_VM_ptr();
}
void guin_API_print_main_stack(guin_VM* vm)
{
    if (!vm) return;
    for (guin_ValueHeader* ptr = vm->stack_main.stackptr-1; ptr >= vm->stack_main.baseptr; --ptr) {
        printf("Datatype: %s %d", guin_GINSTR_Datatype_to_string(ptr->current_value_type), ptr->current_value_type);
        switch (ptr->current_value_type)
        {
            case GINSTRDATATYPE_NULL:       putchar('\n'); break;
            case GINSTRDATATYPE_NIL:        putchar('\n'); break;
            case GINSTRDATATYPE_STRING:     guin_printf(" -> \"%s\"\n", ptr->str); break;
            case GINSTRDATATYPE_INT32:      guin_printf(" -> %d\n", ptr->i32); break;
            case GINSTRDATATYPE_INT64:      guin_printf(" -> %ld\n", ptr->i64); break;
            case GINSTRDATATYPE_NUMBER32:   guin_printf(" -> %f\n", ptr->n32); break;
            case GINSTRDATATYPE_NUMBER64:   guin_printf(" -> %lf\n", ptr->n64); break;
            case GINSTRDATATYPE_BOOL:       guin_printf(" -> %b\n", ptr->bl); break;
            case GINSTRDATATYPE_CHAR:       guin_printf(" -> '%c'\n", ptr->ch); break;
            case GINSTRDATATYPE_FUNCTION:   guin_printf(" -> function\n"); break;
            case GINSTRDATATYPE_DYNAMIC:    guin_printf(" -> dynamic\n"); break;
            default:                        guin_printf(" -> error\n"); break;
        }
    }
}
void guin_API_print_global(guin_VM* vm)
{
    if (!vm) return;
    if (!vm->global.content) return;
    for (size_t i = 0; i < vm->global.length; ++i) {
        printf("%s: %s", vm->global.content[i].name, guin_GINSTR_Datatype_to_string(vm->global.content[i].datatype));
        switch (vm->global.content[i].ptr_to_value->current_value_type)
        {
            case GINSTRDATATYPE_NULL:       puts(" -> null"); break;
            case GINSTRDATATYPE_NIL:        puts(" -> nil"); break;
            case GINSTRDATATYPE_STRING:     guin_printf(" -> \"%s\"\n", vm->global.content[i].ptr_to_value->str); break;
            case GINSTRDATATYPE_INT32:      guin_printf(" -> %d\n", vm->global.content[i].ptr_to_value->i32); break;
            case GINSTRDATATYPE_INT64:      guin_printf(" -> %ld\n", vm->global.content[i].ptr_to_value->i64); break;
            case GINSTRDATATYPE_NUMBER32:   guin_printf(" -> %f\n", vm->global.content[i].ptr_to_value->n32); break;
            case GINSTRDATATYPE_NUMBER64:   guin_printf(" -> %lf\n", vm->global.content[i].ptr_to_value->n64); break;
            case GINSTRDATATYPE_BOOL:       guin_printf(" -> %b\n", vm->global.content[i].ptr_to_value->bl); break;
            case GINSTRDATATYPE_CHAR:       guin_printf(" -> '%c'\n", vm->global.content[i].ptr_to_value->ch); break;
            case GINSTRDATATYPE_FUNCTION:   guin_printf(" -> function\n"); break;
            case GINSTRDATATYPE_DYNAMIC:    guin_printf(" -> dynamic\n"); break;
            default:                        guin_printf(" -> error\n"); break;
        }
    }
}
//bool guin_API_bootstrap_VM_w_file(guin_VM* vm, FILE* file);
//bool guin_API_bootstrap_VM_w_dir(guin_VM* vm, const char* dir);
//const char* guin_API_extract_error(guin_VM* vm);
void guin_API_close_VM(guin_VM** vm)
{
    guin_destroy_VM_ptr(vm);
}

//bool guin_API_declare_global_VM(guin_VM* vm, const char* global_name);

//bool guin_API_load_global(guin_VM* vm, const char* global_name);
//bool guin_API_load_field(guin_VM* vm);
//bool guin_API_load_index(guin_VM* vm, guin_API_Value value);
//bool guin_API_push_load(guin_VM* vm);
//bool guin_API_write_to_load(guin_VM* vm);

// used in order to signify when the end of the function args are in the stack
//bool guin_API_push_FUNC_ARG_START(guin_VM* vm);
bool guin_API_push_IMMEDIATE(guin_VM* vm, guin_API_Value value)
{
    printf("getting val\n");
    guin_ValueHeader x = guin_API_Value_to_ValueHeader(value);
    printf("start\n");
    return guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
}
//bool guin_API_push_GLOBAL(guin_VM* vm, const char* global_name);

// pops 2 vals, returns 1 val
bool guin_API_ADD(guin_VM* vm)
    { return guin_VM_add(vm); }
bool guin_API_SUB(guin_VM* vm)
    { return guin_VM_sub(vm); }
bool guin_API_MUL(guin_VM* vm)
    { return guin_VM_mul(vm); }
bool guin_API_DIV(guin_VM* vm)
    { return guin_VM_div(vm); }
bool guin_API_MOD(guin_VM* vm)
    { return guin_VM_mod(vm); }
bool guin_API_POW(guin_VM* vm)
    { return guin_VM_pow(vm); }
// pops 1, returns 1 val
bool guin_API_NEG(guin_VM* vm)
    { return guin_VM_neg(vm); }

// pops 1, returns 1 val
bool guin_API_NOT(guin_VM* vm)
    { return guin_VM_not(vm); }
bool guin_API_AND(guin_VM* vm)
    { return guin_VM_and(vm); }
bool guin_API_OR(guin_VM* vm)
    { return guin_VM_or(vm); }
bool guin_API_EQU(guin_VM* vm)
    { return guin_VM_equ(vm); }
bool guin_API_NOT_EQU(guin_VM* vm)
    { return guin_VM_not_equ(vm); }
bool guin_API_LT(guin_VM* vm)
    { return guin_VM_lt(vm); }
bool guin_API_GT(guin_VM* vm)
    { return guin_VM_gt(vm); }
bool guin_API_LT_EQU(guin_VM* vm)
    { return guin_VM_lt_equ(vm); }
bool guin_API_GT_EQU(guin_VM* vm)
    { return guin_VM_gt_equ(vm); }
// you have to push all the function args correctly, pops 1 (for func)
//bool guin_API_FUNC_CALL(guin_VM* vm);
// does the same as guin_API_FUNC_CALL, but it guarantees it wont push 
// something onto the stack once done (the ret val)
//bool guin_API_FUNC_CALL_NO_RET(guin_VM* vm);

#endif
