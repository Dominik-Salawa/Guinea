#ifndef guin_LOG_h
#define guin_LOG_h

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

extern size_t guin_log_layer_size;
extern bool guin_log_on;
#define guin_log_pop_layer()  --guin_log_layer_size
#define guin_log_push_layer() ++guin_log_layer_size
void guin_log(char* string, ...);
void guin_log_raw(char* string, ...);
void guin_log_write(void* origin, size_t size, size_t number, FILE* fd);

#endif
