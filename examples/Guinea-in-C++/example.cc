#include <iostream>
#include "../../include/guinea.h"

int main()
{
    GUIN_VM* vm = GUIN_API_open_VM();
    GUIN_API_push_IMMEDIATE(vm, GUIN_VALUE('a'));
    GUIN_API_push_IMMEDIATE(vm, GUIN_VALUE(102));
    GUIN_API_ADD(vm);
    GUIN_API_push_IMMEDIATE(vm, GUIN_VALUE(nullptr));
    GUIN_API_NOT(vm);
    GUIN_API_push_IMMEDIATE(vm, GUIN_VALUE("Hello,"));
    GUIN_API_push_IMMEDIATE(vm, GUIN_VALUE(" "));    // concatenates in front of "Hello,"
    GUIN_API_push_IMMEDIATE(vm, GUIN_VALUE("World! ")); // concatenates in front of " "
    std::cout << "Before:\n";
    GUIN_API_print_main_stack(vm);
    GUIN_API_ADD(vm);
    GUIN_API_ADD(vm);
    GUIN_API_push_IMMEDIATE(vm, GUIN_VALUE(5));
    GUIN_API_MUL(vm);
    std::cout << "\nAfter:\n";
    GUIN_API_print_main_stack(vm);
    GUIN_API_close_VM(&vm);
    std::cout << "\nDone with the program runtime!" << std::endl;
    return 0;
}