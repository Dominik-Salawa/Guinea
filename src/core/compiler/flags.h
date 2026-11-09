#ifndef guin_FLAGS_h
#define guin_FLAGS_h

#include <stdio.h>
#include <stdbool.h>

typedef struct guin_Flags {
	bool debug;
	bool shadow;
} guin_Flags;
typedef void (*guin_Flag_enable)(guin_Flags*);

typedef struct guin_Flag_option {
	const char* long_ver;
	const char* short_ver;
	const guin_Flag_enable enabler;
} guin_Flag_option;

long guin_fprint_Flags(FILE* f, const guin_Flags* flags); 
long guin_print_Flags(const guin_Flags* flags);
bool guin_set_compiler_flag(const char* arg, guin_Flags* fo);

extern guin_Flag_option guin_flag_options[];

#endif
