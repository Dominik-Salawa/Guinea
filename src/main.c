#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "etc/log.c"
#include "etc/strings.c"
#include "etc/string_manipulation.c"
#include "etc/stdlib/G_stdio.c"

#include "core/ver.h"

#include "core/interpreter/value.c"
#include "core/bytecode.c"
#include "core/errors.c"

#include "core/interpreter/stacks.c"
#include "core/interpreter/runtime.c"

#include "core/compiler/ast.c"
#include "core/compiler/lexer.c"
#include "core/compiler/parser.c"
#include "core/compiler/ir.c"

int main(int argc, char** argv)
{
    int i = 1;

    if (argc == 1) {
        printf("Guinea v%d.%d.%d\n", MAJOR_VER, MINOR_VER, BUGFIX_VER);
        return 0;
    }

    if (argc > 2) {
        if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0) {
            ++i;
            G_log_on = true;
        }
    }

    G_ubyte mode = 0;

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

    //printf("%s\n", argv[i]);

    if (!f) {
        printf("Failed to open file!\n");
        return 1;
    }

    
    String inp = readfile(f);
    fclose(f);

    if (!inp.content || inp.length == 0) return 0;

    G_IR ir = (G_IR){0};
    ir.filename = argv[i];
    ir.source = inp;

    G_Bytecode* x = G_IR_CONVERT(&ir, true);

    if (x) {
        print_G_Bytecode_into_G_ASM(x);
        freopen("file.gs", "w", stdout);
        print_G_Bytecode_into_G_ASM(x);

        FILE* tosave = fopen("file.gbc", "w");
        if (tosave) {
            fwrite(x->bytecode, sizeof(G_ubyte), x->length, tosave);
            fclose(tosave);
        }
    }

    destroy_G_Bytecode_ptr(&x);
    clearstring(&inp);

    return 0;
}
