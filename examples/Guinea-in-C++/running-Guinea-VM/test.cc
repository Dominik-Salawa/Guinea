#include <iostream>
#include "../../../src/guinea.h"

const char* filename = "../../../examples/file.gn";

int main(int argc, char** argv)
{
    GUIN_VM* vm = GUIN_init_VM_ptr();

    std::FILE* f = fopen(filename, "r");
    if (!f) { std::cout << "failed to open file " << filename << "!\n"; return 1; }

    GUIN_IR ir;
    {
        ir.filename = new char[strlen(filename)];
        strcpy(ir.filename, filename);
        ir.ver[0] = GUIN_MAJOR_VER;
        ir.ver[1] = GUIN_MINOR_VER;
        ir.source = GUIN_readfile(f);
        if (!ir.source.content) return 1;
    }
    fclose(f);

    //GUIN_log_on = true;
    GUIN_Bytecode* bc = GUIN_IR_CONVERT(&ir, true);
    if (!bc) return 1;

    GUIN_print_Bytecode_into_ASM(bc);

    delete[] ir.filename;
    GUIN_clearstring(&ir.source);
    GUIN_load_Bytecode_into_VM(vm, bc);
    GUIN_run_VM(vm);

    std::cout << "\nGlobal variables:\n";
    GUIN_API_print_global(vm);

    std::cout << "\nStack:\n";
    GUIN_API_print_main_stack(vm);

    if (vm->errmsg) std::cout << "\nGuinea: " << vm->errmsg << '\n';
    GUIN_destroy_VM_ptr(&vm);
    return 0;
}