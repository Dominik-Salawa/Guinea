#ifndef guin_COMPILER_h
#define guin_COMPILER_h

#include "flags.h"

typedef struct guin_Compiler {
	char* filename;
	guin_Flags flags;

	guin_ParseState pState;
	guin_IR ir;
} guin_Compiler;

#endif
