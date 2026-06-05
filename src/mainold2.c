#include <stdio.h>
#include "etc/strings.h"
#include "etc/input.h"

#include "core/lexer.c"
#include "core/errors.c"

int main()
{
    FILE* f = fopen("file.g", "r");
    if (!f) {
        printf("Failed to open file\n"); return 1;
    }
    String str = readfile(f);

    std_err_message(stdout, "file.g", str.content, "This code sucks ass icl", 4, 4, 5);

    return 0;
}