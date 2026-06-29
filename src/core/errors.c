#ifndef ERRORS_C
#define ERRORS_C

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "errors.h"
#include "../etc/strings.h"
#include "../etc/color.h"

bool point_to_error(FILE* f, char* source, size_t line, size_t column, size_t len)
{
    String error_line = init_String();

    size_t current_line = 1;
    size_t i = 0;

    while (current_line != line) {
        char ch = source[i++];

        if (ch == '\0') break;

        if (ch == '\n') {
            ++current_line;
            if (current_line == line)
                break;
        }
    }

    if (current_line == line) {
        while (source[i] != '\n' && source[i] != '\0')
            stringaddchar(&error_line, source[i++]);

        { // Print the text
            fprintf(f, COLOR_WHITE"   |  "COLOR_CLEAR);
            size_t current_column = 1;
            for (; current_column <= error_line.length; current_column++) {
                if (current_column == column)
                    fprintf(f, COLOR_ERR);
                else if (current_column == column + len)
                    fprintf(f, COLOR_CLEAR);
                
                putc(error_line.content[current_column-1], f);
            }
            putc('\n', f);
        }

        { // Print the "^^^^" below
            fprintf(f, COLOR_WHITE"   |  "COLOR_CLEAR);
            size_t current_column = 1;
            while (current_column < column) {
                char ch = (error_line.content[current_column-1] == '\t') ? '\t' : ' ';
                putc(ch, f);
                ++current_column;
            }

            fprintf(f, COLOR_ERR);
            while (current_column < column + len) {
                putc('^', f);
                ++current_column;
            }
            fprintf(f, COLOR_CLEAR);
            putc('\n', f);
        }
    }
    
    clearstring(&error_line);
    return current_line == line;
}

bool std_err_message(FILE* f, char* filename, char* source, char* message, size_t line, size_t column, size_t len)
{
    int printlen = fprintf(f, COLOR_WHITE"COMPILER ERROR: %s:%zu:%zu | %s\n" , filename, line, column, message)-1;
    for (int i = 0; i < printlen-7; ++i) putc('-',f);
    fprintf(f, COLOR_CLEAR);
    putc('\n',f);
    bool exit = point_to_error(f, source, line, column, len);
    fprintf(f, COLOR_WHITE"1 error generated.\n"COLOR_CLEAR);
    return exit;
}

#endif