#ifndef GUIN_GC_H
#define GUIN_GC_H

#include <stdlib.h>
#include "value.h"
#include "vm.h"

const size_t GUIN_MEM_PAGE_SIZE = 1024;

typedef int GC_STATUS;

#define GUIN_GC_STATUS_SUCCESS     0
#define GUIN_GC_STATUS_MEM_FAIL    1

#define GUIN_GC_MARKED   1
#define GUIN_GC_UNMARKED 0

typedef struct GUIN_MEM_PAGE {
    struct GUIN_MEM_PAGE* prev;
    struct GUIN_MEM_PAGE* next;
    GUIN_ValueHeader* page;
    size_t vacant_slots; // if == 0 then skip it
} GUIN_MEM_PAGE;
GUIN_MEM_PAGE* GUIN_init_MEM_PAGE_ptr(void);
void GUIN_destroy_MEM_PAGE_ptr(GUIN_MEM_PAGE** x);

const size_t GUIN_initial_GC_page_count = 3;
typedef struct GUIN_GC {
    GUIN_MEM_PAGE* pages;
    size_t page_count;
} GUIN_GC;
GUIN_GC GUIN_init_GC(void);
void GUIN_destroy_GC(GUIN_GC* gc);

// returns whether the GC needed to make memory for the required slots (if 0 then it wont make any)
GC_STATUS GUIN_GC_activate(GUIN_VM* vm, size_t required_slots);
// marks every ValueHeader in every page for sweep
void GUIN_GC_mark(GUIN_VM* vm);
// returns whether the GC needed to make memory for the required slots (if 0 then it wont make any)
// and assumes GUIN_GC_mark has been called beforehand
GC_STATUS GUIN_GC_sweep(GUIN_GC* gc, size_t required_slots);

// NULL == memory fault and you should abort unless required_slots == 0
// else it will return the exact size of the required slots youve asked
// which you have to free the list later
GUIN_ValueHeader** GUIN_GC_get_memory(GUIN_VM* vm, size_t required_slots);

#endif
