#ifndef GUINEA_STDIO_C
#define GUINEA_STDIO_C

#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "G_stdio.h"
#include "../strings.h"
#include "../declarations.h"

ssize_t G_vfmt(char* buffer_addr, size_t buffer_size, char* string, va_list args)
{
    bool no_write = buffer_addr && buffer_size;

    // finish

    return 0;
}

ssize_t G_fmt(char* buffer_addr, size_t buffer_size, char* string, ...)
{
    va_list args;
    va_start(args, string);
    ssize_t size = G_vfmt(buffer_addr, buffer_size, string, args);
    va_end(args);
    return size;
}

size_t G_vprintf(FILE* file, char* string, va_list args)
{
    ssize_t amount_of_chars_outputted = 0; 
    while (true) {
        char ch = *string;
        if (ch == 0) break;

        if (ch == '%') {
            ch = *++string;
                
            switch (ch) 
            {
                case '%': {
                    putc('%', file);
                    ++amount_of_chars_outputted;
                    break;
                }

                case 'b': {
                    amount_of_chars_outputted += fprintf(file, (va_arg(args, int) != 0)? "true" : "false");
                }

                case 'c': {
                    putc(va_arg(args, int), file);
                    ++amount_of_chars_outputted;
                    break;
                }

                case 's': {
                    ch = *++string;
                    if (ch == 'c') {
                        char* ptr = va_arg(args, char*);
                        amount_of_chars_outputted += fprintf(file, "%s", (!ptr)? "(null)" : ptr);
                    } else {
                        String str = va_arg(args, String);
                        amount_of_chars_outputted += fwrite(str.content, sizeof(typeof(*str.content)), str.length, file);
                        --string;
                    }
                    break;
                }

                case 'p': {
                    amount_of_chars_outputted += fprintf(file, "%p", va_arg(args, void*));
                    break;
                }

                case 'd': {
                    amount_of_chars_outputted += fprintf(file, "%d", va_arg(args, G_int32));
                    break;
                }

                case 'u': {
                    amount_of_chars_outputted += fprintf(file, "%u", va_arg(args, G_int32));
                    break;
                }

                case 'x': {
                    amount_of_chars_outputted += fprintf(file, "%x", va_arg(args, G_int32));
                    break;
                }

                case 'o': {
                    amount_of_chars_outputted += fprintf(file, "%o", va_arg(args, G_int32));
                    break;
                }

                case 'f': {
                    amount_of_chars_outputted += fprintf(file, "%f", va_arg(args, G_number64));
                    break;
                }

                case 'l': {
                    ch = *++string;
                    if (ch == 'd') {
                        amount_of_chars_outputted += fprintf(file, "%lld", va_arg(args, G_int64));
                    }
                    else if (ch == 'u') {
                        amount_of_chars_outputted += fprintf(file, "%llu", va_arg(args, G_int64));
                    }
                    else if (ch == 'x') {
                        amount_of_chars_outputted += fprintf(file, "%llx", va_arg(args, G_int64));
                    }
                    else if (ch == 'o') {
                        amount_of_chars_outputted += fprintf(file, "%llo", va_arg(args, G_int64));
                    }
                    else if (ch == 'f') {
                        amount_of_chars_outputted += fprintf(file, "%lf", va_arg(args, G_number64));
                    }
                    else {
                        fprintf(file, "G_vprintf error: invalid format (%%l%c)!\n", ch);
                        exit(1);
                    }
                    break;
                }

                case 'z': {
                    ch = *++string;
                    if (ch == 'd') {
                        amount_of_chars_outputted += fprintf(file, "%zd", va_arg(args, size_t));
                    }
                    else if (ch == 'u') {
                        amount_of_chars_outputted += fprintf(file, "%zu", va_arg(args, size_t));
                    }
                    else if (ch == 'x') {
                        amount_of_chars_outputted += fprintf(file, "%zx", va_arg(args, size_t));
                    }
                    else if (ch == 'o') {
                        amount_of_chars_outputted += fprintf(file, "%zo", va_arg(args, size_t));
                    }
                    else {
                        fprintf(file, "G_vprintf error: invalid format (%%z%c)!\n", ch);
                        exit(1);
                    }
                    break;
                }

                default:
                    return -1;
            }
        } 
        else {
            putc(ch, file);
            ++amount_of_chars_outputted;
        }

        ++string;
    }
    return amount_of_chars_outputted;
}

size_t G_fprintf(FILE* file, char* string, ...)
{
    va_list args;
    va_start(args, string);
    size_t amount_of_chars_outputted = G_vprintf(file, string, args);
    va_end(args);
    return amount_of_chars_outputted;
}

size_t G_printf(char* string, ...)
{
    va_list args;
    va_start(args, string);
    size_t amount_of_chars_outputted = G_vprintf(stdout, string, args);
    va_end(args);
    return amount_of_chars_outputted;
}




String input()
{
    String str = init_String();
    if (!str.content) return (String){0};

    int ch;
    while ((ch = fgetc(stdin)) > EOF && ch != 10) {
        if (str.length >= str.size-1) {
            str.size *= 2;
            str.content = realloc(str.content, str.size);

            if (!str.content) {
                str.size    = 0;
                str.length  = 0;
                return str;
            }
        }
        str.content[str.length++] = ch;
    }

    str.content[str.length] = 0;
    return str;
}

String readfile(FILE* file)
{
    String str = init_String();
    if (!str.content) return (String){0};

    int ch;
    while ((ch = fgetc(file)) > EOF) {
        if (str.length >= str.size-1) {
            str.size *= 2;
            str.content = realloc(str.content, str.size);

            if (!str.content) {
                str.size    = 0;
                str.length  = 0;
                return str;
            }
        }
        str.content[str.length++] = ch;
    }

    str.content[str.length] = 0;
    return str;
}



#endif