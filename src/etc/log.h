#ifndef LOG_H
#define LOG_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "log.h"

size_t GUIN_log_layer_size = 0;
bool GUIN_log_on = false;
#define GUIN_log_pop_layer()  --GUIN_log_layer_size
#define GUIN_log_push_layer() ++GUIN_log_layer_size
void GUIN_log(char* string, ...);
void GUIN_log_raw(char* string, ...);
void GUIN_log_write(void* origin, size_t size, size_t number, FILE* fd);

#endif
