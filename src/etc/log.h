#ifndef LOG_H
#define LOG_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
size_t G_log_layer_size = 0;
bool G_log_on = false;
void G_log(char* string, ...);
void G_log_raw(char* string, ...);
void G_log_write(void* origin, size_t size, size_t number, FILE* fd);

#define G_log_pop_layer()  --G_log_layer_size
#define G_log_push_layer() ++G_log_layer_size

#endif