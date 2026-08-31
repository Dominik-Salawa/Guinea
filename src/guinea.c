#ifndef GUIN_C
#define GUIN_C

// THIS FILES ENTIRE PURPOSE IS TO QUICKLY LINK AND INCLUDE ALL THE .c FILES FOR GUINEA
// /include/guinea.h DOES NOT SERVE THE SAME PURPOSE AND IT IS JUST FOR EMBEDDING API
// guinea.h is located in /include/

#include "../include/guinea.h"
#include "etc/stdlib/GUIN_stdio.h"

#include "declarations.c"

#include "etc/log.c"
#include "etc/strings.c"
#include "etc/string_manipulation.c"
#include "etc/stdlib/GUIN_stdio.c"

#include "core/compiler.c"
#include "core/interpreter.c"

#include "core/ver.h"

GUIN_API_Value GUIN_API_nil(void* _)                            { return (GUIN_API_Value){.type=GUIN_API_DT_NIL};           }
GUIN_API_Value GUIN_API_int16(GUIN_int16 data)                  { return (GUIN_API_Value){.type=GUIN_API_DT_I16,.i16=data}; }
GUIN_API_Value GUIN_API_int32(GUIN_int32 data)                  { return (GUIN_API_Value){.type=GUIN_API_DT_I32,.i32=data}; }
GUIN_API_Value GUIN_API_int64(GUIN_int64 data)                  { return (GUIN_API_Value){.type=GUIN_API_DT_I64,.i64=data}; }
GUIN_API_Value GUIN_API_number32(GUIN_number32 data)            { return (GUIN_API_Value){.type=GUIN_API_DT_N32,.n32=data}; }
GUIN_API_Value GUIN_API_number64(GUIN_number64 data)            { return (GUIN_API_Value){.type=GUIN_API_DT_N64,.n64=data}; }
GUIN_API_Value GUIN_API_char(char data)                         { return (GUIN_API_Value){.type=GUIN_API_DT_CHAR,.ch=data}; }
GUIN_API_Value GUIN_API_bool(bool data)                         { return (GUIN_API_Value){.type=GUIN_API_DT_BOOL,.bl=data}; }
GUIN_API_Value GUIN_API_string(const char* data)                { return (GUIN_API_Value){.type=GUIN_API_DT_STR,.str=data}; }

GUIN_ValueHeader GUIN_API_Value_to_ValueHeader(GUIN_API_Value x)
{
    switch (x.type)
    {
        case GUIN_API_DT_NIL:  return (GUIN_ValueHeader){.header_type=GINSTRDATATYPE_NIL,.current_value_type=GINSTRDATATYPE_NIL};
        case GUIN_API_DT_I16:  return (GUIN_ValueHeader){.i32=x.i16,.header_type=GINSTRDATATYPE_INT32,.current_value_type=GINSTRDATATYPE_INT32};
        case GUIN_API_DT_I32:  return (GUIN_ValueHeader){.i32=x.i32,.header_type=GINSTRDATATYPE_INT32,.current_value_type=GINSTRDATATYPE_INT32};
        case GUIN_API_DT_I64:  return (GUIN_ValueHeader){.i64=x.i64,.header_type=GINSTRDATATYPE_INT64,.current_value_type=GINSTRDATATYPE_INT64};
        case GUIN_API_DT_N32:  return (GUIN_ValueHeader){.n32=x.n32,.header_type=GINSTRDATATYPE_NUMBER32,.current_value_type=GINSTRDATATYPE_NUMBER32};
        case GUIN_API_DT_N64:  return (GUIN_ValueHeader){.n64=x.n64,.header_type=GINSTRDATATYPE_NUMBER64,.current_value_type=GINSTRDATATYPE_NUMBER64};
        case GUIN_API_DT_CHAR: return (GUIN_ValueHeader){.ch=x.ch,.header_type=GINSTRDATATYPE_CHAR,.current_value_type=GINSTRDATATYPE_CHAR};
        case GUIN_API_DT_BOOL: return (GUIN_ValueHeader){.bl=x.bl,.header_type=GINSTRDATATYPE_BOOL,.current_value_type=GINSTRDATATYPE_BOOL};
        case GUIN_API_DT_STR: {
            GUIN_String* s = GUIN_init_String_ptr();
            GUIN_stringconcat_charptr(s, x.str);
            return (GUIN_ValueHeader){.str=s,.header_type=GINSTRDATATYPE_STRING,.current_value_type=GINSTRDATATYPE_STRING};
        }
        default: return (GUIN_ValueHeader){0};
    }
}

GUIN_VM* GUIN_API_open_VM(void)
{
    return GUIN_init_VM_ptr();
}
void GUIN_API_print_main_stack(GUIN_VM* vm)
{
    if (!vm) return;
    for (GUIN_ValueHeader* ptr = vm->stack_main.stackptr-1; ptr >= vm->stack_main.baseptr; --ptr) {
        printf("Datatype: %s", GUIN_GINSTR_Datatype_to_string(ptr->current_value_type));
        switch (ptr->current_value_type)
        {
            case GINSTRDATATYPE_NULL:       putchar('\n'); break;
            case GINSTRDATATYPE_NIL:        putchar('\n'); break;
            case GINSTRDATATYPE_STRING:     GUIN_printf(" -> \"%s\"\n", ptr->str); break;
            case GINSTRDATATYPE_INT32:      GUIN_printf(" -> %d\n", ptr->i32); break;
            case GINSTRDATATYPE_INT64:      GUIN_printf(" -> %ld\n", ptr->i64); break;
            case GINSTRDATATYPE_NUMBER32:   GUIN_printf(" -> %f\n", ptr->n32); break;
            case GINSTRDATATYPE_NUMBER64:   GUIN_printf(" -> %lf\n", ptr->n64); break;
            case GINSTRDATATYPE_BOOL:       GUIN_printf(" -> %b\n", ptr->bl); break;
            case GINSTRDATATYPE_CHAR:       GUIN_printf(" -> '%c'\n", ptr->ch); break;
            case GINSTRDATATYPE_FUNCTION:   GUIN_printf(" -> function\n"); break;
            case GINSTRDATATYPE_DYNAMIC:    GUIN_printf(" -> dynamic\n"); break;
            default:                        GUIN_printf(" -> error\n"); break;
        }
    }
}
void GUIN_API_print_global(GUIN_VM* vm)
{
    if (!vm) return;
    if (!vm->global.content) return;
    for (size_t i = 0; i < vm->global.length; ++i) {
        printf("%s: %s", vm->global.content[i].name, GUIN_GINSTR_Datatype_to_string(vm->global.content[i].datatype));
        switch (vm->global.content[i].ptr_to_value->current_value_type)
        {
            case GINSTRDATATYPE_NULL:       puts(" -> null"); break;
            case GINSTRDATATYPE_NIL:        puts(" -> nil"); break;
            case GINSTRDATATYPE_STRING:     GUIN_printf(" -> \"%s\"\n", vm->global.content[i].ptr_to_value->str); break;
            case GINSTRDATATYPE_INT32:      GUIN_printf(" -> %d\n", vm->global.content[i].ptr_to_value->i32); break;
            case GINSTRDATATYPE_INT64:      GUIN_printf(" -> %ld\n", vm->global.content[i].ptr_to_value->i64); break;
            case GINSTRDATATYPE_NUMBER32:   GUIN_printf(" -> %f\n", vm->global.content[i].ptr_to_value->n32); break;
            case GINSTRDATATYPE_NUMBER64:   GUIN_printf(" -> %lf\n", vm->global.content[i].ptr_to_value->n64); break;
            case GINSTRDATATYPE_BOOL:       GUIN_printf(" -> %b\n", vm->global.content[i].ptr_to_value->bl); break;
            case GINSTRDATATYPE_CHAR:       GUIN_printf(" -> '%c'\n", vm->global.content[i].ptr_to_value->ch); break;
            case GINSTRDATATYPE_FUNCTION:   GUIN_printf(" -> function\n"); break;
            case GINSTRDATATYPE_DYNAMIC:    GUIN_printf(" -> dynamic\n"); break;
            default:                        GUIN_printf(" -> error\n"); break;
        }
    }
}
//bool GUIN_API_bootstrap_VM_w_file(GUIN_VM* vm, FILE* file);
//bool GUIN_API_bootstrap_VM_w_dir(GUIN_VM* vm, const char* dir);
//const char* GUIN_API_extract_error(GUIN_VM* vm);
void GUIN_API_close_VM(GUIN_VM** vm)
{
    GUIN_destroy_VM_ptr(vm);
}

//bool GUIN_API_declare_global_VM(GUIN_VM* vm, const char* global_name);

//bool GUIN_API_load_global(GUIN_VM* vm, const char* global_name);
//bool GUIN_API_load_field(GUIN_VM* vm);
//bool GUIN_API_load_index(GUIN_VM* vm, GUIN_API_Value value);
//bool GUIN_API_push_load(GUIN_VM* vm);
//bool GUIN_API_write_to_load(GUIN_VM* vm);

// used in order to signify when the end of the function args are in the stack
//bool GUIN_API_push_FUNC_ARG_START(GUIN_VM* vm);
bool GUIN_API_push_IMMEDIATE(GUIN_VM* vm, GUIN_API_Value value)
{
    printf("getting val\n");
    GUIN_ValueHeader x = GUIN_API_Value_to_ValueHeader(value);
    printf("start\n");
    return GUIN_add_VALUE_to_VALUE_STACK(&vm->stack_main, x);
}
//bool GUIN_API_push_GLOBAL(GUIN_VM* vm, const char* global_name);

// pops 2 vals, returns 1 val
bool GUIN_API_ADD(GUIN_VM* vm)
    { return GUIN_VM_add(vm); }
bool GUIN_API_SUB(GUIN_VM* vm)
    { return GUIN_VM_sub(vm); }
bool GUIN_API_MUL(GUIN_VM* vm)
    { return GUIN_VM_mul(vm); }
bool GUIN_API_DIV(GUIN_VM* vm)
    { return GUIN_VM_div(vm); }
bool GUIN_API_MOD(GUIN_VM* vm)
    { return GUIN_VM_mod(vm); }
bool GUIN_API_POW(GUIN_VM* vm)
    { return GUIN_VM_pow(vm); }
// pops 1, returns 1 val
bool GUIN_API_NEG(GUIN_VM* vm)
    { return GUIN_VM_neg(vm); }

// pops 1, returns 1 val
bool GUIN_API_NOT(GUIN_VM* vm)
    { return GUIN_VM_not(vm); }
bool GUIN_API_AND(GUIN_VM* vm)
    { return GUIN_VM_and(vm); }
bool GUIN_API_OR(GUIN_VM* vm)
    { return GUIN_VM_or(vm); }
bool GUIN_API_EQU(GUIN_VM* vm)
    { return GUIN_VM_equ(vm); }
bool GUIN_API_NOT_EQU(GUIN_VM* vm)
    { return GUIN_VM_not_equ(vm); }
bool GUIN_API_LT(GUIN_VM* vm)
    { return GUIN_VM_lt(vm); }
bool GUIN_API_GT(GUIN_VM* vm)
    { return GUIN_VM_gt(vm); }
bool GUIN_API_LT_EQU(GUIN_VM* vm)
    { return GUIN_VM_lt_equ(vm); }
bool GUIN_API_GT_EQU(GUIN_VM* vm)
    { return GUIN_VM_gt_equ(vm); }
// you have to push all the function args correctly, pops 1 (for func)
//bool GUIN_API_FUNC_CALL(GUIN_VM* vm);
// does the same as GUIN_API_FUNC_CALL, but it guarantees it wont push 
// something onto the stack once done (the ret val)
//bool GUIN_API_FUNC_CALL_NO_RET(GUIN_VM* vm);

#endif
