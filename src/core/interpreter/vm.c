#ifndef guin_VM_c
#define guin_VM_c

#include "stacks.h"
#include "vm.h"
#include "vm.h"
#include "../../../include/declarations.h"
#include <math.h>
#include <string.h>

guin_GLOBALMAP guin_init_GLOBALMAP(void)
{
    guin_GLOBALMAP x;
    x.size = 8;
    x.length = 0;
    x.content = guin_malloc(sizeof(guin_VARIABLE_HEADER) * x.size);
    if (!x.content) return (guin_GLOBALMAP){0};
    return x;
}
void guin_destroy_GLOBALMAP(guin_GLOBALMAP* x)
{
    if (!x) return;
    if (x->content) {
        for (size_t i = 0; i < x->length; ++i)
            guin_destroy_VARIABLE_HEADER(&x->content[i]);
        guin_mfree(x->content);
    }
    *x = (guin_GLOBALMAP){0};
}
// tales ownership of global_name
guin_STATUS guin_add_GLOBALNAME_to_GLOBALMAP(guin_GLOBALMAP* x, GINSTR_Datatype datatype, char* global_name)
{
    if (!x || datatype == GINSTRDATATYPE_NULL || !global_name) return guin_FAIL;
    if (guin_fetch_GLOBALNAME_from_GLOBALMAP(x, global_name))  return guin_FAIL;

    if (x->length >= x->size) {
        size_t original_size = x->size;
        if (x->size > 0) {
            while (x->length >= x->size) x->size *= 2;
        } else {
            x->size = 8;
        }
        guin_VARIABLE_HEADER* tmp = guin_remalloc(x->content, sizeof(guin_VARIABLE_HEADER) * x->size);
        if (!tmp) {
            x->size = original_size;
            return guin_MEM_FAIL;
        }
        x->content = tmp;
    }

    guin_VARIABLE_HEADER y = guin_init_VARIABLE_HEADER(datatype, global_name, NULL);
    if (!y.name) return guin_MEM_FAIL;
    x->content[x->length++] = y;
    return guin_SUCCESS;
}
guin_STATUS guin_add_GLOBALNAME_to_GLOBALMAP_cp_global_name(guin_GLOBALMAP* x, GINSTR_Datatype datatype, char* global_name)
{
    if (!x || datatype == GINSTRDATATYPE_NULL || !global_name) return guin_FAIL;
    if (guin_fetch_GLOBALNAME_from_GLOBALMAP(x, global_name))  return guin_FAIL;

    if (x->length >= x->size) {
        size_t original_size = x->size;
        if (x->size > 0) {
            while (x->length >= x->size) x->size *= 2;
        } else {
            x->size = 8;
        }
        guin_VARIABLE_HEADER* tmp = guin_remalloc(x->content, sizeof(guin_VARIABLE_HEADER) * x->size);
        if (!tmp) {
            x->size = original_size;
            return guin_MEM_FAIL;
        }
        x->content = tmp;
    }

    guin_VARIABLE_HEADER y = guin_init_VARIABLE_HEADER_cp_name(datatype, global_name, NULL);
    if (!y.name) return guin_MEM_FAIL;
    x->content[x->length++] = y;
    return guin_SUCCESS;
}
// DO NOT guin_mfree
guin_VARIABLE_HEADER* guin_fetch_GLOBALNAME_from_GLOBALMAP(guin_GLOBALMAP* x, char* global_name)
{
    if (!x || !global_name) return NULL;

    for (size_t i = 0; i < x->length; ++i) {
        if (strcmp(x->content[i].name, global_name) == 0)
            return &x->content[i];
    }
    return NULL;
}



guin_VM guin_init_VM(void)
{
    guin_VM x = (guin_VM){0};
    x.stack_main = guin_init_VALUE_STACK();
    if (!x.stack_main.baseptr) {
        return (guin_VM){0};
    }
    x.stack_frames = guin_init_FRAME_STACK();
    if (!x.stack_frames.baseptr) {
        guin_destroy_VALUE_STACK(&x.stack_main);
        return (guin_VM){0};
    }
    return x;
}
guin_VM* guin_init_VM_ptr(void)
{
    guin_VM* x = guin_malloc(sizeof(guin_VM));
    if (!x) return NULL;
    *x = guin_init_VM();
    if (!x->stack_main.baseptr) // means something went wrong and the entire value got zeroed
        return NULL;
    return x;
}
void guin_destroy_VM(guin_VM* x)
{
    if (!x) return;
    guin_destroy_VALUE_STACK(&x->stack_main);
    guin_destroy_FRAME_STACK(&x->stack_frames);
    *x = (guin_VM){0};
}
void guin_destroy_VM_ptr(guin_VM** x)
{
    if (!x) return;
    if (!*x) return;
    guin_destroy_VM(*x);
    guin_mfree(*x);
    *x = NULL;
}




guin_STATUS GINSTR_VM_PUSH_IMMEDIATE_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
    ++frame->pc;
    guin_VH_from_BC_result x = guin_get_ValueHeader_from_Bytecode(&frame->func->funcval->bytecode, frame->pc);
    if (x.status == guin_FAIL) {
        guin_printf("%sc\n", x.errmsg);
        guin_destroy_ValueHeader(&x.value);
        return guin_FAIL;
    }
    if (x.status == guin_MEM_FAIL) {
        guin_destroy_ValueHeader(&x.value);
        return guin_MEM_FAIL;
    }

    bool s = guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, x.value);
    if (!s) {
        guin_destroy_ValueHeader(&x.value);
        return s;
    }
    frame->pc += x.to_jump;
    return guin_SUCCESS;
}
guin_STATUS GINSTR_VM_DECLARE_GLOBAL_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
    ++frame->pc;
    GINSTR_Datatype dt = *frame->pc;
    ++frame->pc;
    guin_uint64 namelen;
    if (!guin_safe_memcpy_Bytecode(&namelen, &frame->func->funcval->bytecode, frame->pc, sizeof(namelen))) {
        return guin_CORRUPTED_DATA;
    }
    frame->pc += sizeof(namelen);
    char* name = guin_malloc(namelen + 1);
    if (!name) return guin_MEM_FAIL;

    for (size_t i = 0; i < (size_t)namelen; ++i, ++frame->pc)
        name[i] = *frame->pc;
    name[namelen] = 0;

    guin_STATUS s = guin_add_GLOBALNAME_to_GLOBALMAP(&vm->global, dt, name);
    if (s != guin_SUCCESS) {
        guin_mfree(name);
        return s;
    }

    { // assigns the Global variable its own dedicated slot and assigns from stack
        guin_ValueHeader** x = guin_GC_get_memory(vm, 1);
        if (!x) return guin_MEM_FAIL;

        
        guin_VARIABLE_HEADER* glocation = guin_fetch_GLOBALNAME_from_GLOBALMAP(&vm->global, name);
        glocation->ptr_to_value = *x;
        glocation->ptr_to_value->header_type = dt;
        glocation->ptr_to_value->current_value_type = GINSTRDATATYPE_NIL;
        guin_mfree(x);

        guin_ValueHeader popped_val = guin_pop_VALUE_STACK(&vm->stack_main);
		printf("got location %p %s\n", (void*)glocation->ptr_to_value, guin_GINSTR_Datatype_to_string(popped_val.header_type));
        s = guin_assign_ValueHeader_with_ValueHeader(glocation->ptr_to_value, popped_val);
        if (s != guin_SUCCESS) {
            printf("unsuccessful assignment to `%s`\n", name);
            if (popped_val.current_value_type != GINSTRDATATYPE_NULL)
                guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, popped_val);
            return s;
        }
    }
    return guin_SUCCESS;
}

#define guin_VM_is_valid_Program_Counter(pc, bytecode) (pc >= bytecode.bytecode && pc < bytecode.bytecode + bytecode.length)

guin_STATUS GINSTR_VM_POP_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
	++frame->pc;
	return (guin_pop_VALUE_STACK(&vm->stack_main).header_type != GINSTRDATATYPE_NULL)? guin_SUCCESS : guin_FAIL;
}

guin_STATUS GINSTR_VM_JMP_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
    frame->pc += 1;
    guin_ubyte jmp;

    if (!guin_safe_memcpy_Bytecode(&jmp, &frame->func->funcval->bytecode, frame->pc, sizeof(guin_ubyte))) {
        vm->errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
        return guin_FAIL;
    }

    frame->pc += *(frame->pc - 1);
    return guin_SUCCESS;
}
guin_STATUS GINSTR_VM_JMPL_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
    frame->pc += 1;
    guin_JUMPL_SIZE jmp;

    if (!guin_safe_memcpy_Bytecode(&jmp, &frame->func->funcval->bytecode, frame->pc, sizeof(guin_JUMPL_SIZE))) {
        vm->errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
        return guin_FAIL;
    }

    frame->pc += jmp + sizeof(guin_JUMPL_SIZE);
    return guin_SUCCESS;
}

guin_STATUS GINSTR_VM_RET_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
    if (frame->no_ret && frame->func->funcval->return_type != GINSTRDATATYPE_VOID)
        guin_pop_VALUE_STACK(&vm->stack_main);

    guin_pop_FRAME_STACK(&vm->stack_frames);    
    return guin_SUCCESS;
}

guin_STATUS GINSTR_VM_LOAD_GLOBAL_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
	guin_uint64 len;
	++frame->pc;
	if (!guin_safe_memcpy_Bytecode(&len, &frame->func->funcval->bytecode, frame->pc, sizeof(guin_uint64))) {
        vm->errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
        return guin_FAIL;
    }

	frame->pc += sizeof(guin_uint64);

	char name[len+1];
	name[len] = 0;
	if (!guin_safe_memcpy_Bytecode(&name, &frame->func->funcval->bytecode, frame->pc, len)) {
        vm->errmsg = "Encountered corrupted bytecode mid-way through the runtime!";
        return guin_FAIL;
    }

	guin_VARIABLE_HEADER* n = guin_fetch_GLOBALNAME_from_GLOBALMAP(&vm->global, name);
	if (!n) {
		vm->errmsg = "Failed to fetch global name!";
		return guin_FAIL;
	}

	frame->pc += len;
	vm->load = n->ptr_to_value;
	printf("val: %ld %s\n", n->ptr_to_value->i64, name);
    return guin_SUCCESS;
}

guin_STATUS GINSTR_VM_WRITE_LOAD_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
	if (!vm->load) {
		vm->errmsg = "Failed to Write, VM load is NULL!";
		return guin_FAIL;
	}

	guin_ValueHeader vh = guin_pop_VALUE_STACK(&vm->stack_main);
	if (vh.header_type == GINSTRDATATYPE_NULL) {
		vm->errmsg = "Failed to Write, Main Stack is empty!";
		return guin_FAIL;
	}

	guin_STATUS s = guin_assign_ValueHeader_with_ValueHeader(vm->load, vh);
	if (s != guin_SUCCESS) return s;

	++frame->pc;
	return guin_SUCCESS;
}

guin_STATUS GINSTR_VM_PUSH_LOAD_BYTECODE(guin_VM* vm, guin_FRAME* frame)
{
	if (!vm->load) {
		vm->errmsg = "Failed to Push, VM load is NULL!";
		return guin_FAIL;
	}

	guin_ValueHeader l = *vm->load;
	if (!guin_add_VALUE_to_VALUE_STACK(&vm->stack_main, l))
		return guin_MEM_FAIL;

	++frame->pc;
	return guin_SUCCESS;
}

typedef guin_STATUS (*guin_BytecodeHandler)(guin_VM* vm, guin_FRAME* frame);
static guin_BytecodeHandler guin_VM_bytecode_handler[256] = {
    [GINSTR_PUSH_IMMEDIATE] = &GINSTR_VM_PUSH_IMMEDIATE_BYTECODE,
    [GINSTR_DECLARE_GLOBAL] = &GINSTR_VM_DECLARE_GLOBAL_BYTECODE,

    [GINSTR_JMP]            = &GINSTR_VM_JMP_BYTECODE,
    [GINSTR_JMPL]           = &GINSTR_VM_JMPL_BYTECODE,

    [GINSTR_LOAD_GLOBAL]    = &GINSTR_VM_LOAD_GLOBAL_BYTECODE,
    [GINSTR_WRITE_LOAD]     = &GINSTR_VM_WRITE_LOAD_BYTECODE,
    [GINSTR_PUSH_LOAD]      = &GINSTR_VM_PUSH_LOAD_BYTECODE,
    [GINSTR_PUSH_IREG]      = &GINSTR_VM_PUSH_IREG,
    [GINSTR_POP]            = &GINSTR_VM_POP_BYTECODE,
    [GINSTR_RET]            = &GINSTR_VM_RET_BYTECODE,
};


// DOES NOT DEEP COPY
guin_STATUS guin_load_Bytecode_into_VM(guin_VM* vm, guin_Bytecode* global)
{
    if (!vm || !global) return guin_FAIL;

    guin_FunctionValue* func = guin_malloc(sizeof(guin_FunctionValue));
    func->argc = 0;
	guin_Bytecode b;
	b.bytecode = guin_malloc(global->length);
	if (!b.bytecode) {
		guin_mfree(func);
		return guin_MEM_FAIL;
	}
	b.length = global->length;
	b.size   = global->length;
	for (size_t i = 0; i < global->length; ++i)
		b.bytecode[i] = global->bytecode[i];

    func->bytecode = b;
    func->return_type = GINSTRDATATYPE_VOID;

    guin_ValueHeader** y = guin_GC_get_memory(vm, 1);
	if (!y) {
		guin_destroy_Bytecode(&b);
		guin_mfree(func);
		return guin_MEM_FAIL;
	}
    guin_ValueHeader* x = *y;
    guin_mfree(y);

    x->current_value_type = GINSTRDATATYPE_FUNCTION;
    x->funcval = func;
    guin_FRAME frame = guin_init_FRAME(x);

    if (!frame.pc)
        return guin_FAIL;

    return (guin_add_FRAME_to_FRAME_STACK(&vm->stack_frames, frame))? guin_SUCCESS : guin_MEM_FAIL;
}


guin_STATUS guin_run_VM(guin_VM* vm)
{
    if (!vm) return guin_FAIL;
    vm->errmsg = NULL;
    for (guin_FRAME* frame = vm->stack_frames.stackptr-1; vm->stack_frames.stackptr != vm->stack_frames.baseptr; frame = vm->stack_frames.stackptr-1) {
        guin_Bytecode* bc = &frame->func->funcval->bytecode;

        if (guin_overflow_Bytecode(bc, frame->pc)) {
            vm->errmsg = "Program Counter overflow!";
            return guin_FAIL;
        }
            
        if (guin_underflow_Bytecode(bc, frame->pc)) {
            vm->errmsg = "Program Counter underflow!";
            return guin_FAIL;
        }
            
        guin_BytecodeHandler func = guin_VM_bytecode_handler[*frame->pc];
        if (func == NULL) {
            vm->errmsg = "Unknown bytecode!";
            //printf("Error byte: %d %zu\n", *frame->pc, (size_t)(frame->pc - frame->func->funcval->bytecode.bytecode));
            return guin_FAIL;
        }
        guin_STATUS s = func(vm, frame);
        if (s != guin_SUCCESS) return s;
    }
    return guin_SUCCESS;
}

// true:  loaded and did execute (but not necessarily there was no runtime errors)
// false: failed to load/execute
guin_STATUS guin_exec_VM(guin_VM* vm, char* start)
{
    guin_VARIABLE_HEADER* start_header = guin_fetch_GLOBALNAME_from_GLOBALMAP(&vm->global, start);
    if (!start_header) return guin_FAIL;
    if (start_header->datatype != GINSTRDATATYPE_FUNCTION || !start_header->ptr_to_value) return guin_FAIL;

    return guin_SUCCESS;
}

#endif
