#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "etc/log.c"
#include "etc/strings.c"
#include "etc/string_manipulation.c"
#include "etc/stdlib/G_stdio.c"

#include "core/ver.h"

#include "core/value.c"
#include "core/bytecode.c"
#include "core/errors.c"

#include "core/interpreter/runtime.c"

#include "core/compiler/ast.c"
#include "core/compiler/lexer.c"
#include "core/compiler/parser.c"
#include "core/compiler/ir.c"


// 1. Forward-declare the exact Windows types and functions manually
typedef struct _EXCEPTION_RECORD {
    unsigned long ExceptionCode;
    // (Other OS fields exist here, but we only need to map the layout sizes)
    unsigned long ExceptionFlags;
    struct _EXCEPTION_RECORD* ExceptionRecord;
    void* ExceptionAddress;
    unsigned long NumberParameters;
    unsigned __int64 ExceptionInformation[15];
} EXCEPTION_RECORD, *PEXCEPTION_RECORD;

typedef struct _CONTEXT CONTEXT, *PCONTEXT;

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

typedef long (__stdcall* PVECTORED_EXCEPTION_HANDLER)(PEXCEPTION_POINTERS);

// 2. Link the core Windows DLL functions directly
__declspec(dllimport) void* __stdcall AddVectoredExceptionHandler(
    unsigned long First, 
    PVECTORED_EXCEPTION_HANDLER Handler
) __attribute__((dllimport)); // Attribute helps MinGW link correctly

__declspec(dllimport) int __stdcall MessageBoxA(
    void* hWnd, 
    const char* lpText, 
    const char* lpCaption, 
    unsigned int uType
) __attribute__((dllimport));

// 3. Define the crash watcher callback
long __stdcall CustomCrashWatcher(PEXCEPTION_POINTERS ExceptionInfo) {
    // 0xC0000005 is the Windows NT status code for an Access Violation (Segfault)
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == 0xC0000005) {
        // 0x00000010L maps to MB_ICONERROR | MB_OK
        MessageBoxA(NULL, "Application crashed with a Segmentation Fault!", "Segfault Intercepted", 0x00000010L);
    }
    return 1; // 1 maps to EXCEPTION_EXECUTE_HANDLER (allows normal exit after popup)
}

int main(int argc, char** argv)
{
    AddVectoredExceptionHandler(1, CustomCrashWatcher);
    size_t i = 1;

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

    for (; i < argc; i++) {
        FILE* f = fopen(argv[i], "r");

        //printf("%s\n", argv[i]);

        if (!f) {
            printf("Failed to open file!\n");
            return 1;
        }

        
        String inp = readfile(f);

        if (!inp.content || inp.length == 0) return 0;

        G_IR ir = (G_IR){0};
        ir.filename = argv[i];
        ir.source = inp;

        G_Bytecode* x = G_IR_CONVERT(&ir, sizeof(size_t), true);

        if (x) {
            for (size_t i = 0; i < x->length; i++) {
                printf("%d ", x->bytecode[i]);
            }
            putchar(10);

            //freopen("file.gs", "w", stdout);
            print_G_Bytecode_into_G_ASM(x);

            freopen("file.gs", "w", stdout);
            print_G_Bytecode_into_G_ASM(x);

            FILE* tosave = fopen("file.gbc", "w");
            if (tosave) {
                fwrite(x->bytecode, sizeof(G_ubyte), x->length, tosave);
                fclose(tosave);
            }
        }

        //FILE* save = fopen("file.gbc", "w");
        //if (save) fwrite(x->bytecode, sizeof(G_ubyte), x->length, save);

        destroy_G_Bytecode_ptr(&x);
        clearstring(&inp);
    }

    return 0;
}