#include <stdio.h>
#include <stdlib.h>

#include "guinea.c"

int main(int argc, char** argv)
{
    int i = 1;

    if (argc == 1) {
        printf("Guinea v%d.%d.%d\n", GUIN_MAJOR_VER, GUIN_MINOR_VER, GUIN_BUGFIX_VER);
        return 0;
    }

    if (argc > 2) {
        if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0) {
            ++i;
            GUIN_log_on = true;
        }
    }

    GUIN_ubyte mode = 0;

    if (argc > i + 1) {
        if (strcmp(argv[i], "run") == 0) {
            mode = 1;
        } else if (strcmp(argv[i], "compile") == 0) {
            mode = 2;
        }
    }
    ++i;
    
    if (mode == 0) {
        printf("Error: didnt say a valid method on what to do!\n");
        return 1;
    }

    FILE* f = fopen(argv[i], "r");

    if (!f) {
        printf("Failed to open file!\n");
        return 1;
    }

    
    GUIN_String inp = GUIN_readfile(f);
    fclose(f);

    if (!inp.content || inp.length == 0) return 0;

    GUIN_IR ir = (GUIN_IR){0};
    ir.filename = argv[i];
    ir.source = inp;

    GUIN_Bytecode* x = GUIN_IR_CONVERT(&ir, true);

    if (x) {
        GUIN_print_Bytecode_into_ASM(x);
        if (mode == 1) {

        }
        else if (mode == 2) {
            ++i;
            if (i < argc) {
                FILE* tosave = fopen(argv[i], "w");
                if (tosave) {
                    fwrite(x->bytecode, sizeof(GUIN_ubyte), x->length, tosave);
                    fclose(tosave);
                }
            } else {
                printf("no place to compile it to!\n");
            }
        }
    }

    GUIN_destroy_Bytecode_ptr(&x);
    GUIN_clearstring(&inp);
    return 0;
}
