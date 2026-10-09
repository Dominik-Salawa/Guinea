#ifndef guin_GC_c
#define guin_GC_c

#include "gc.h"

guin_MEM_PAGE* guin_init_MEM_PAGE_ptr(void)
{
    guin_MEM_PAGE* x = malloc(sizeof(guin_MEM_PAGE));
    if (!x) return NULL;

    *x = (guin_MEM_PAGE){0};
    x->page = malloc(sizeof(guin_ValueHeader) * guin_MEM_PAGE_SIZE);
    if (!x->page) {
        free(x);
        return NULL;
    }
    x->vacant_slots = guin_MEM_PAGE_SIZE;
    return x;
}
void guin_destroy_MEM_PAGE_ptr(guin_MEM_PAGE** x)
{
    for (size_t i = 0; i < guin_MEM_PAGE_SIZE; ++i)
        guin_destroy_ValueHeader(&(*x)->page[i]);
    free(*x);
    *x = NULL;
}




guin_GC guin_init_GC(void)
{
    guin_GC x = (guin_GC){0};

    for (size_t i = 0; i < guin_initial_GC_page_count; ++i) {
        guin_MEM_PAGE* page = guin_init_MEM_PAGE_ptr();
        if (!page) {
            guin_MEM_PAGE* current = x.pages;
            while (current) {
                guin_MEM_PAGE* next = current->next;
                guin_destroy_MEM_PAGE_ptr(&current);
                current = next;
            }
            return (guin_GC){0};
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
void guin_destroy_GC(guin_GC* x)
{
    guin_MEM_PAGE* current = x->pages;
    while (current) {
        guin_MEM_PAGE* next = current->next;
        guin_destroy_MEM_PAGE_ptr(&current);
        current = next;
    }
    x->page_count = 0;
}

// returns whether the GC needed to make memory for the required slots (if 0 then it wont make any)
GC_STATUS guin_GC_activate(guin_VM* vm, size_t required_slots)
{
    if (!vm) return false;
    guin_GC_mark(vm);
    return guin_GC_sweep(&vm->gc, required_slots);
}



// for guin_GC_mark
static void guin_GC_mark_ValueHeader(guin_ValueHeader* v)
{
    if (!v) return;
    v->GC_status = guin_GC_MARKED;
}

// marks every ValueHeader in every page for sweep
void guin_GC_mark(guin_VM* vm)
{
    for (size_t i = 0; i < vm->global.length; ++i) {
        guin_GC_mark_ValueHeader(vm->global.content[i].ptr_to_value);
    }
    for (guin_ValueHeader* ptr = vm->stack_main.baseptr; ptr < vm->stack_main.stackptr; ++ptr) {
        guin_GC_mark_ValueHeader(ptr);
    }
}
// returns whether the GC needed to make memory for the required slots (if 0 then it wont make any)
// and assumes guin_GC_mark has been called beforehand
GC_STATUS guin_GC_sweep(guin_GC* gc, size_t required_slots)
{
    size_t original_required_slots = required_slots;

    guin_MEM_PAGE* last = NULL;
    for (guin_MEM_PAGE* current = gc->pages; current != NULL; last = current, current = current->next) {
        bool passes = false;
        
        if (required_slots != 0 && current->vacant_slots != 0) {
            required_slots -= (current->vacant_slots <= required_slots)? current->vacant_slots : required_slots;
            passes = true;
        }

        if (current->vacant_slots == 0)
            goto assess_page;

        for (size_t i = 0; i < guin_MEM_PAGE_SIZE; ++i) {
            guin_ValueHeader* header = &current->page[i];

            if (header->current_value_type == GINSTRDATATYPE_NULL)
                continue;
            if (header->GC_status == guin_GC_UNMARKED) {
                --current->vacant_slots;
                if (required_slots != 0) --required_slots;
                guin_destroy_ValueHeader(header);
            }
        }
        
        assess_page: if (!passes) {
            if (current->prev) current->prev->next = current->next;
            if (current->next) current->next->prev = current->prev;
            guin_MEM_PAGE* next = current->next;
            guin_destroy_MEM_PAGE_ptr(&current);
            current = next;
        }
    }
    while (required_slots != 0) {
        guin_MEM_PAGE* page = guin_init_MEM_PAGE_ptr();
        if (!page)
            return guin_GC_STATUS_MEM_FAIL;
        last->next = page;
        ++gc->page_count;
        last = page;
        required_slots -= (guin_MEM_PAGE_SIZE <= required_slots)? guin_MEM_PAGE_SIZE : required_slots;
    }
    return guin_GC_STATUS_SUCCESS;
}

// NULL == memory fault and you should abort unless required_slots == 0
// else it will return the exact size of the required slots youve asked
// which you have to free the list later
guin_ValueHeader** guin_GC_get_memory(guin_VM* vm, size_t required_slots)
{
    if (!vm || required_slots == 0) return NULL;
    guin_GC* gc = &vm->gc;

    guin_ValueHeader** x = malloc(sizeof(guin_ValueHeader*) * required_slots);
    if (!x) return NULL;

    size_t x_len = 0;

    search: {
        bool done = false;
        
        for (guin_MEM_PAGE* current = gc->pages; current != NULL && !done; current = current->next) {
            if (current->vacant_slots == 0) continue;

            for (size_t i = 0; i < guin_MEM_PAGE_SIZE; ++i) {
                guin_ValueHeader* header = &current->page[i];
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
            GC_STATUS status = guin_GC_activate(vm, required_slots);

            if (status == guin_GC_STATUS_MEM_FAIL) {
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
