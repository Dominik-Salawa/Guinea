#ifndef guin_GC_h
#define guin_GC_h

#include <stdlib.h>
#include "value.h"

typedef struct guin_VM guin_VM;

const size_t guin_MEM_PAGE_SIZE = 1024;

typedef int guin_GC_STATUS;

#define guin_GC_STATUS_SUCCESS     0
#define guin_GC_STATUS_MEM_FAIL    1

#define guin_GC_MARKED   1
#define guin_GC_UNMARKED 0

typedef struct guin_MEM_PAGE {
    struct guin_MEM_PAGE* prev;
    struct guin_MEM_PAGE* next;
    guin_ValueHeader* page;
    size_t vacant_slots; // if == 0 then skip it
} guin_MEM_PAGE;
guin_MEM_PAGE* guin_init_MEM_PAGE_ptr(void);
void guin_destroy_MEM_PAGE_ptr(guin_MEM_PAGE** x);

const size_t guin_initial_GC_page_count = 3;
typedef struct guin_GC {
    guin_MEM_PAGE* pages;
    size_t page_count;
} guin_GC;
guin_GC guin_init_GC(void);
void guin_destroy_GC(guin_GC* gc);

// returns whether the GC needed to make memory for the required slots (if 0 then it wont make any)
guin_GC_STATUS guin_GC_activate(guin_VM* vm, size_t required_slots);
// marks every ValueHeader in every page for sweep
void guin_GC_mark(guin_VM* vm);
// returns whether the GC needed to make memory for the required slots (if 0 then it wont make any)
// and assumes guin_GC_mark has been called beforehand
guin_GC_STATUS guin_GC_sweep(guin_GC* gc, size_t required_slots);

// NULL == memory fault and you should abort unless required_slots == 0
// else it will return the exact size of the required slots youve asked
// which you have to free the list later
guin_ValueHeader** guin_GC_get_memory(guin_VM* vm, size_t required_slots);

#endif
