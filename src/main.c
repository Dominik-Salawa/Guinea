#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "guinea.c"

static inline bool Advance_CLI(int* i, const char** argv, const int argc) 
{
	if (*i == argc-1) return false;
	int ii = *i + 1;
	while (argv[ii][0] == '-' && ii < argc-1) ++ii;
	int old = *i;
	*i = ii;
	return old != ii && argv[ii][0] != '-';
}

// only used inside int main(...)
#define A() Advance_CLI(&i, argv, argc)

int main(const int argc, const char** argv, const char** envp)
{
    if (argc == 1) {
        printf("Guinea v%d.%d.%d\n", guin_MAJOR_VER, guin_MINOR_VER, guin_BUGFIX_VER);
        return 0;
    }

	guin_Compiler compiler = {0};

	for (size_t i = 1; i < (size_t)argc; ++i) {
		if (argv[i][0] == '-' && !guin_set_compiler_flag(argv[i], &compiler.flags)) {
			printf("Guinea: `%s` is an unknown flag!\n", argv[i]);
			return 1;
		}
	}
    
	int i = 0;
	A();
	
	const char* task = argv[i];

	if (strcmp(task, "run") == 0) {
		if (!A()) {
			printf("Guinea: Expected a file directory to run!\n");
			return 1;
		}

		guin_String s;
		{
			FILE* f = fopen(argv[i], "r");
			if (!f) {
				printf("Guinea: Failed to open `%s`!\n", argv[i]);
				return 1;
			}

			s = guin_readfile(f);
			fclose(f);
			if (!s.content) {
				printf("Guinea: Failed to read `%s`!\n", argv[i]);
				return 1;
			}
		}

		guin_IR ir = {.filename="test",.source=s};

		guin_Bytecode* bc = guin_IR_CONVERT(&ir);
		guin_clearstring(&s);
		if (!bc) {
			printf("Guinea: Failed to convert `%s`!\n", argv[i]);
			return 1;
		}

		guin_VM vm = guin_init_VM();
		guin_load_Bytecode_into_VM(&vm, bc);
		guin_run_VM(&vm);

		guin_print_Bytecode_into_ASM(bc);
		guin_API_print_global(&vm);

		putchar(10);

		guin_API_print_main_stack(&vm);

		if (vm.errmsg) puts(vm.errmsg);

		guin_destroy_VM(&vm);
		guin_clearstring(&s);
	} 
	else if (strcmp(task, "compile") == 0) {
		if (!A()) {
			printf("Guinea: Expected a file directory to compile!\n");
			return 1;
		}

		guin_String s;
		{
			FILE* f = fopen(argv[i], "r");
			if (!f) {
				printf("Guinea: Failed to open `%s`!\n", argv[i]);
				return 1;
			}

			s = guin_readfile(f);
			fclose(f);
			if (!s.content) {
				printf("Guinea: Failed to read `%s`!\n", argv[i]);
				return 1;
			}
		}

		if (!A()) {
			printf("Guinea: Expected a file directory to save to!\n");
			guin_clearstring(&s);
			return 1;
		}
		
		guin_VM vm = guin_init_VM();

		guin_destroy_VM(&vm);
		guin_clearstring(&s);
	} 
	else {
		printf((argv[i][0] != '-')? "Guinea: `%s` is an unknown task!\n" : "Guinea: No task was given!\n", task);
		return 1;
	}



    return 0;
}
