#include <stdio.h>
#include "etc/strings.c"
#include "etc/string_manipulation.c"
#include "etc/input.c"

#include "core/value.c"
#include "core/compiler/lexer.c"
#include "core/compiler/parser.c"
#include "core/compiler/ast.c"
#include "core/errors.c"

int main()
{
    FILE* f = fopen("file.g", "r");

    if (!f) {
        printf("Failed to open file!\n");
        return 1;
    }

    String inp = readfile(f);

    if (!inp.content || inp.length == 0) return 0;

    parse(&inp);

    /*
    LexToken l = {0};
    LexState lState = init_LexState(&inp);

    while(l.type != TK_EOF) {
        l = advance_lexer(&lState);
        if (l.type == TK_Identifier)
            printf("%s %s\n", LexTokenEnum_to_string(l.type), l.string.content);
        else
            printf("%s\n", LexTokenEnum_to_string(l.type));
    }
    */

    clearstring(&inp);

    return 0;
}