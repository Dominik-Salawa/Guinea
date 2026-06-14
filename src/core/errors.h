#ifndef ERRORS_H
#define ERRORS_H

#include <stdlib.h>
#include <stdio.h>
bool point_to_error(FILE* f, char* source, size_t line, size_t column, size_t len);
bool std_err_message(FILE* f, char* filename, char* source, char* message, size_t line, size_t column, size_t len);

#endif