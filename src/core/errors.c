#ifndef ERRORS_C
#define ERRORS_C

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "errors.h"
#include "../etc/strings.h"
#include "../etc/color.h"

bool GUIN_ERROR_point_to_error(FILE* f, char* source, char* msg_next_to_ptr, size_t line, size_t column, size_t len)
{
    GUIN_String error_line = GUIN_init_String();

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
            GUIN_stringaddchar(&error_line, source[i++]);

        size_t length_of_num;
        { // Print the text
            length_of_num = fprintf(f, "  "GUIN_COLOR_ERR"%zu"GUIN_COLOR_CLEAR" ", line) - (strlen(GUIN_COLOR_ERR) + strlen(GUIN_COLOR_CLEAR));
            fprintf(f, GUIN_COLOR_WHITE"|  "GUIN_COLOR_CLEAR);
            size_t current_column = 1;
            for (; current_column <= error_line.length; current_column++) {
                if (current_column == column)
                    fprintf(f, GUIN_COLOR_ERR);
                else if (current_column == column + len)
                    fprintf(f, GUIN_COLOR_CLEAR);
                
                putc(error_line.content[current_column-1], f);
            }
            putc('\n', f);
        }

        { // Print the "^^^^" below
            fprintf(f, GUIN_COLOR_WHITE);
            for (i = 0; i < length_of_num; ++i) putc(' ', f);
            fprintf(f, "|  "GUIN_COLOR_CLEAR);
            size_t current_column = 1;
            while (current_column < column) {
                char ch = (error_line.content[current_column-1] == '\t') ? '\t' : ' ';
                putc(ch, f);
                ++current_column;
            }

            fprintf(f, GUIN_COLOR_ERR);
            while (current_column < column + len) {
                putc('^', f);
                ++current_column;
            }
            if (msg_next_to_ptr) {
                fprintf(f, " %s"GUIN_COLOR_CLEAR"\n", msg_next_to_ptr);
            } else {
                fprintf(f, GUIN_COLOR_CLEAR"\n");
            }
        }
    }
    
    GUIN_clearstring(&error_line);
    return current_line == line;
}

bool GUIN_ERROR_std_err_message(FILE* f, char* filename, char* source, char* message, char* msg_next_to_ptr, size_t line, size_t column, size_t len)
{
    int printlen = fprintf(f, GUIN_COLOR_WHITE"GUINEA ERROR: %s:%zu:%zu | %s\n" , filename, line, column, message)-1;
    for (int i = 0; i < printlen-7; ++i) putc('-',f);
    fprintf(f, GUIN_COLOR_CLEAR);
    putc('\n',f);
    bool exit = GUIN_ERROR_point_to_error(f, source, msg_next_to_ptr, line, column, len);
    fprintf(f, GUIN_COLOR_WHITE"1 error generated.\n"GUIN_COLOR_CLEAR);
    return exit;
}

#endif
