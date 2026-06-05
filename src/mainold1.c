#include <stdio.h>
#include "etc/strings.h"
#include "etc/input.h"

#include "core/value.c"
#include "core/lexer.c"
#include "core/parser.c"
#include "core/errors.c"

int main()
{
    //printf("SUPPORT FLOATS/DOUBLES, OPERATIONS LIKE ==, !=, ETC..., AND CHARS, THEN MAKE PARSER WORK!\n");

    //while (1) {
    FILE* f = fopen("file.g", "r");

    if (!f) {
        printf("Failed to open file!\n");
        return 1;
    }

    String inp = readfile(f);
    //String inp = input();

    if (!inp.content || inp.length == 0) return 0;

    LexState lState = init_LexState(&inp);

    LexToken l;
    while ((l = advance_lexer(&lState)).type != TK_EOF) {
        if (l.type == TK_Identifier) {
            printf("[%s] ", l.string.content);
            clearstring(&l.string);
        } else if (l.type == TK_String_val) {
            printf("\"%s\" ",  l.string.content);
            clearstring(&l.string);
        } else if (l.type == TK_Number_val)
            printf("{%g} ", l.number);
        else if (l.type == TK_Int_val)
            printf("(%lld) ", l.integer);
        else if (l.type == TK_Bool_val) {
            printf("%s ", (l.bl == true) ? "true" : "false");
        } else if (l.type == TK_EOF) {
            printf("EOF\n");
            break;
        } else {
            if (l.type == TK_UNKNOWN) {
                printf("\n");
                std_err_message(stdout, "file.g", inp.content, lState.errmsg, l.line, l.column, l.string.length);
                clearstring(&l.string);
            } else if (l.type == TK_ERR) {
                printf("\n");
                std_err_message(stdout, "file.g", inp.content, lState.errmsg, l.line, l.column, l.string.length);
                clearstring(&l.string);
            } else
                printf("%s ", LexTokenEnum_to_string(l.type));
        }
    }
    printf("\n");
    //printf("\nLAST TOKEN: %s %d:%d\n", LexTokenEnum_to_string(l.type), l.line, l.column);
    //l = advance_lexer(&lState);
    //printf("LAST LAST TOKEN: %s %d:%d\n", LexTokenEnum_to_string(l.type), l.line, l.column);
    //}

    return 0;
}