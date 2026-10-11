#ifndef guin_FLAGS_c
#define guin_FLAGS_c

#include <stdio.h>
#include "flags.h"

long guin_fprint_Flags(FILE* f, const guin_Flags* flags) 
	{ return guin_fprintf(f, "| debug: %b\n", flags->debug); }
long guin_print_Flags(const guin_Flags* flags)
	{ return guin_fprint_Flags(stdout, flags); }

void guin_debug_enabler(guin_Flags* flags) { flags->debug = true; }

guin_Flag_option guin_flag_options[] = {
	{.long_ver="--debug",.short_ver="-d",.enabler=guin_debug_enabler}
};

bool guin_set_compiler_flag(const char* arg, guin_Flags* fo)
{
	for (size_t i = 0; i < sizeof(guin_flag_options)/sizeof(guin_Flag_option); ++i) {
		if (strcmp(arg, guin_flag_options[i].long_ver) == 0 || strcmp(arg, guin_flag_options[i].short_ver) == 0){
			guin_flag_options[i].enabler(fo);
			return true;
		}
	}
	return false;
}

#endif
