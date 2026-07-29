#ifndef GUIN_GC_H
#define GUIN_GC_H

#include <stdlib.h>
#include "value.h"

const size_t GUIN_MEM_PAGE_SIZE = 1024;

typedef struct GUIN_MEM_PAGE {
    struct GUIN_MEM_PAGE* prev;
    struct GUIN_MEM_PAGE* next;
    GUIN_ValueHeader* page;
    size_t occupied_slots; // if == PAGE_SIZE then skip it
} GUIN_MEM_PAGE;

typedef struct GUIN_GC {
    GUIN_MEM_PAGE* pages;
    size_t page_count;
    size_t max_page_count_until_GC_sweep; // should be at least 1.2x pages bigger than page_count
} GUIN_GC;

// returns whether the GC needed to make memory for the required slots
bool GUIN_GC_activate(GUIN_GC* gc, size_t required_slots);
// marks every ValueHeader in every page for sweep
void GUIN_GC_mark(GUIN_GC* gc);
// returns whether the GC needed to make memory for the required slots
// and assumes GUIN_GC_mark has been called beforehand
bool GUIN_GC_sweep(GUIN_GC* gc, size_t required_slots);

// NULL == memory fault and you should abort NOW
// else it will return the exact size of the required slots youve asked
// which you have to free the list later
GUIN_ValueHeader** GUIN_GC_get_memory(GUIN_GC* gc, size_t required_slots);

#endif
