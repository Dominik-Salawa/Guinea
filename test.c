#include <stdio.h>

int main()
{
    FILE* f = fopen("testoffile", "w");

    char* c = "GUINEA";

    while (*c) {
        putc(*c++, f);
    }

    int i = 0;
    while (i < 14) {
        putc(4, f);
        i++;
    }

    char* v = "Hello, World!\n";
    while (*v) {
        putc(*v++, f);
    }

    while (i < 67) {
        putc(4, f);
        i++;
    }

    putc('\v', f);


    while (i < 127) {
        putc(4, f);
        i++;
    }

    fclose(f);
    return 0;
}