#ifndef GUIN_GC_C
#define GUIN_GC_C

#include "gc.h"

GUIN_MEM_PAGE* GUIN_init_MEM_PAGE_ptr(void)
{
    GUIN_MEM_PAGE* x = malloc(sizeof(GUIN_MEM_PAGE));
    if (!x) return NULL;

    *x = (GUIN_MEM_PAGE){0};
    x->page = malloc(sizeof(GUIN_ValueHeader) * GUIN_MEM_PAGE_SIZE);
    if (!x->page) {
        free(x);
        return NULL;
    }
    x->vacant_slots = GUIN_MEM_PAGE_SIZE;
    return x;
}
void GUIN_destroy_MEM_PAGE_ptr(GUIN_MEM_PAGE** x)
{
    for (size_t i = 0; i < GUIN_MEM_PAGE_SIZE; ++i)
        GUIN_destroy_ValueHeader(&(*x)->page[i]);
    free(*x);
    *x = NULL;
}




GUIN_GC GUIN_init_GC(void)
{
    GUIN_GC x = (GUIN_GC){0};

    for (size_t i = 0; i < GUIN_initial_GC_page_count; ++i) {
        GUIN_MEM_PAGE* page = GUIN_init_MEM_PAGE_ptr();
        if (!page) {
            GUIN_MEM_PAGE* current = x.pages;
            while (current) {
                GUIN_MEM_PAGE* next = current->next;
                GUIN_destroy_MEM_PAGE_ptr(&current);
                current = next;
            }
            return (GUIN_GC){0};
        }
        if (x.pages) {
            x.pages->prev = page;
            x.pages = page;
        } else {
            x.pages = page;
        }
        ++x.page_count;
    }
    return x;
}
void GUIN_destroy_GC(GUIN_GC* x)
{
    GUIN_MEM_PAGE* current = x->pages;
    while (current) {
        GUIN_MEM_PAGE* next = current->next;
        GUIN_destroy_MEM_PAGE_ptr(&current);
        current = next;
    }
    x->page_count = 0;
}

// returns whether the GC needed to make memory for the required slots (if 0 then it wont make any)
GC_STATUS GUIN_GC_activate(GUIN_VM* vm, size_t required_slots)
{
    if (!vm) return false;
    GUIN_GC_mark(vm);
    return GUIN_GC_sweep(&vm->gc, required_slots);
}



// for GUIN_GC_mark
static void GUIN_GC_mark_ValueHeader(GUIN_ValueHeader* v)
{
    if (!v) return;
    v->GC_status = GUIN_GC_MARKED;
}

// marks every ValueHeader in every page for sweep
void GUIN_GC_mark(GUIN_VM* vm)
{
    for (size_t i = 0; i < vm->global.length; ++i) {
        GUIN_GC_mark_ValueHeader(vm->global.content[i].ptr_to_value);
    }
    for (GUIN_ValueHeader* ptr = vm->stack_main.baseptr; ptr < vm->stack_main.stackptr; ++ptr) {
        GUIN_GC_mark_ValueHeader(ptr);
    }
}
// returns whether the GC needed to make memory for the required slots (if 0 then it wont make any)
// and assumes GUIN_GC_mark has been called beforehand
GC_STATUS GUIN_GC_sweep(GUIN_GC* gc, size_t required_slots)
{
    size_t original_required_slots = required_slots;

    GUIN_MEM_PAGE* last = NULL;
    for (GUIN_MEM_PAGE* current = gc->pages; current != NULL; last = current, current = current->next) {
        bool passes = false;
        
        if (required_slots != 0 && current->vacant_slots != 0) {
            required_slots -= (current->vacant_slots <= required_slots)? current->vacant_slots : required_slots;
            passes = true;
        }

        if (current->vacant_slots == 0)
            goto assess_page;

        for (size_t i = 0; i < GUIN_MEM_PAGE_SIZE; ++i) {
            GUIN_ValueHeader* header = &current->page[i];

            if (header->current_value_type == GINSTRDATATYPE_NULL)
                continue;
            if (header->GC_status == GUIN_GC_UNMARKED) {
                --current->vacant_slots;
                if (required_slots != 0) --required_slots;
                GUIN_destroy_ValueHeader(header);
            }
        }
        
        assess_page: if (!passes) {
            if (current->prev) current->prev->next = current->next;
            if (current->next) current->next->prev = current->prev;
            GUIN_MEM_PAGE* next = current->next;
            GUIN_destroy_MEM_PAGE_ptr(&current);
            current = next;
        }
    }
    while (required_slots != 0) {
        GUIN_MEM_PAGE* page = GUIN_init_MEM_PAGE_ptr();
        if (!page)
            return GUIN_GC_STATUS_MEM_FAIL;
        last->next = page;
        ++gc->page_count;
        last = page;
        required_slots -= (GUIN_MEM_PAGE_SIZE <= required_slots)? GUIN_MEM_PAGE_SIZE : required_slots;
    }
    return GUIN_GC_STATUS_SUCCESS;
}

// NULL == memory fault and you should abort unless required_slots == 0
// else it will return the exact size of the required slots youve asked
// which you have to free the list later
GUIN_ValueHeader** GUIN_GC_get_memory(GUIN_VM* vm, size_t required_slots)
{
    if (!vm || required_slots == 0) return NULL;
    GUIN_GC* gc = &vm->gc;

    GUIN_ValueHeader** x = malloc(sizeof(GUIN_ValueHeader*) * required_slots);
    if (!x) return NULL;

    size_t x_len = 0;

    search: {
        bool done = false;
        
        for (GUIN_MEM_PAGE* current = gc->pages; current != NULL && !done; current = current->next) {
            if (current->vacant_slots == 0) continue;

            for (size_t i = 0; i < GUIN_MEM_PAGE_SIZE; ++i) {
                GUIN_ValueHeader* header = &current->page[i];
                if (header->current_value_type == GINSTRDATATYPE_NULL) {
                    x[x_len++] = header;

                    if (x_len == required_slots) {
                        done = true;
                        break;
                    }
                }
            }
        }
    }

    if (x_len != required_slots) {
        {
            GC_STATUS status = GUIN_GC_activate(vm, required_slots);

            if (status == GUIN_GC_STATUS_MEM_FAIL) {
                free(x);
                return NULL;
            }
        }
        x_len = 0;
        goto search;
    }
    return x;
}

#endif
