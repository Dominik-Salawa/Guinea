#ifndef guin_STDIO_c
#define guin_STDIO_c

#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
#include "stdio.h"
#include "../strings.c"
#include "../../../include/declarations.h"

static long guin_print_double(FILE* file, const guin_number64 num)
{
    long size = snprintf(NULL, 0, "%.15f", num) + 1;
    if (size < 0) return size;

    char buffer[size];

    snprintf(buffer, size, "%.20f", num);

    for (size_t i = size-2; i > 0; --i) {
        if (buffer[i] == '0') {
            buffer[i] = 0;
        }
        else if (buffer[i] == '.') {
            buffer[i] = 0;
            break;
        } 
        else {
            break;
        }
    }
    return fprintf(file, "%s", buffer);
}

long guin_vprintf(FILE* file, char* string, va_list args)
{
    long amount_of_chars_outputted = 0;

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
                    amount_of_chars_outputted += fputs((va_arg(args, int) != false)? "true" : "false", file);
                    break;
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
                    } else if (ch == 's') {
                        guin_String str = va_arg(args, guin_String);
                        if (!str.content)
                            amount_of_chars_outputted += fputs("(null)", file);
                        else
                            amount_of_chars_outputted += fwrite(str.content, sizeof(char), str.length, file);
                    } else {
                        guin_String* str = va_arg(args, guin_String*);
                        if (!str)
                            amount_of_chars_outputted += fputs("(null)", file);
                        else if (!str->content)
                            amount_of_chars_outputted += fputs("(null)", file);
                        else
                            amount_of_chars_outputted += fwrite(str->content, sizeof(char), str->length, file);
                        --string;
                    }
                    break;
                }

                case 'p': {
                    amount_of_chars_outputted += fprintf(file, "%p", va_arg(args, void*));
                    break;
                }

                case 'd': {
                    amount_of_chars_outputted += fprintf(file, "%"PRId32, va_arg(args, guin_int32));
                    break;
                }

                case 'u': {
                    amount_of_chars_outputted += fprintf(file, "%"PRIu32, va_arg(args, guin_int32));
                    break;
                }

                case 'x': {
                    amount_of_chars_outputted += fprintf(file, "%"PRIx32, va_arg(args, guin_int32));
                    break;
                }

                case 'o': {
                    amount_of_chars_outputted += fprintf(file, "%"PRIo32, va_arg(args, guin_int32));
                    break;
                }

                case 'f': {
                    amount_of_chars_outputted += guin_print_double(file, va_arg(args, guin_number64));
                    break;
                }

                case 'l': {
                    ch = *++string;
                    if (ch == 'd') {
                        amount_of_chars_outputted += fprintf(file, "%"PRId64, va_arg(args, guin_int64));
                    }
                    else if (ch == 'u') {
                        amount_of_chars_outputted += fprintf(file, "%"PRIu64, va_arg(args, guin_int64));
                    }
                    else if (ch == 'x') {
                        amount_of_chars_outputted += fprintf(file, "%"PRIx64, va_arg(args, guin_int64));
                    }
                    else if (ch == 'o') {
                        amount_of_chars_outputted += fprintf(file, "%"PRIo64, va_arg(args, guin_int64));
                    }
                    else if (ch == 'f') {
                        amount_of_chars_outputted += guin_print_double(file, va_arg(args, guin_number64));
                    }
                    else {
                        fprintf(file, "guin_vprintf error: invalid format (%%l%c)!\n", ch);
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
                        fprintf(file, "guin_vprintf error: invalid format (%%z%c)!\n", ch);
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

long guin_fprintf(FILE* file, char* string, ...)
{
    va_list args;
    va_start(args, string);
    long amount_of_chars_outputted = guin_vprintf(file, string, args);
    va_end(args);
    return amount_of_chars_outputted;
}

long guin_printf(char* string, ...)
{
    va_list args;
    va_start(args, string);
    long amount_of_chars_outputted = guin_vprintf(stdout, string, args);
    va_end(args);
    return amount_of_chars_outputted;
}




guin_String guin_input(void)
{
    guin_String str = guin_init_String();
    if (!str.content) return (guin_String){0};

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

guin_String guin_readfile(FILE* file)
{
    guin_String str = guin_init_String();
    if (!str.content) return (guin_String){0};

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
