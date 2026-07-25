#ifndef ERRORS_H
#define ERRORS_H

#include <stdlib.h>
#include <stdio.h>
bool GUIN_ERROR_point_to_error(FILE* f, char* source, char* msg_next_to_ptr, size_t line, size_t column, size_t len);
bool GUIN_ERROR_std_err_message(FILE* f, char* filename, char* source, char* message, char* msg_next_to_ptr, size_t line, size_t column, size_t len);

#endif
