#ifndef GUIN_LOG_H
#define GUIN_LOG_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

extern size_t GUIN_log_layer_size;
extern bool GUIN_log_on;
#define GUIN_log_pop_layer()  --GUIN_log_layer_size
#define GUIN_log_push_layer() ++GUIN_log_layer_size
void GUIN_log(char* string, ...);
void GUIN_log_raw(char* string, ...);
void GUIN_log_write(void* origin, size_t size, size_t number, FILE* fd);

#endif
