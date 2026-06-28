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

#define SIZE_T 8

int numlen(size_t number)
{
    int i = 1;
    while (number / 10 != 0) {
        number /= 10;
        ++i;
    }
    return i;
}

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

        G_Bytecode* x = G_IR_CONVERT(&ir, SIZE_T, true);

        if (x) {
            for (size_t i = 0; i < x->length; i++) {
                printf("%d ", x->bytecode[i]);
            }
            putchar(10);

            //freopen("file.gs", "w", stdout);

            int len = 8;
            if (numlen(x->length) > len) {
                len = numlen(x->length);
            }
            for (size_t i = 7; i < x->length;) {
                for (char j = 0; j < len - numlen(i); ++j)
                    putchar('0');

                printf("%zu   ", i-6);

                switch (x->bytecode[i])
                {
                    case GINSTR_DECLARE_GLOBAL: {
                        ++i;
                        GINSTR_Datatype globaldatatype = x->bytecode[i];

                        printf("declare global [%s]: ", G_Bytecode_Datatype_to_str(globaldatatype));

                        ++i;

                        String* name = get_string_in_G_Bytecode(x, i, 8);
                        if (!name) {
                            printf("no name\n");
                            return 1;
                        }

                        i += 8 + name->length; // varname len + int size
                        fwrite(name->content, sizeof(G_ubyte), name->length, stdout);
                        printf("\n");

                        clearstring_ptr(&name);
                        break;
                    }

                    case GINSTR_PUSH_GLOBAL:
                        printf("PUSHGLOBAL\n");
                        ++i;
                        break;

                    case GINSTR_PUSH_IMMEDIATE:
                        printf("push ");
                        ++i;
                        GINSTR_Datatype immediatedatatype = x->bytecode[i];
                        ++i;
                        printf("%s ", G_Bytecode_Datatype_to_str(immediatedatatype));
                        switch (immediatedatatype)
                        {
                            case GINSTRDATATYPE_STRING:
                            {
                                int64_t size;
                                memcpy(&size, &x->bytecode[i], 8);

                                i += 8;

                                String str = init_String();
                                stringconcat_char_w_len(&str, (char*)&x->bytecode[i], size);

                                putchar('"');
                                fwrite(str.content, sizeof(char), size, stdout);
                                putchar('"');
                                i += size;
                                clearstring(&str);
                                break;
                            }

                            case GINSTRDATATYPE_NIL:
                            {
                                break; // dont need to do anything
                            }

                            case GINSTRDATATYPE_INT32:
                            {
                                int32_t val;
                                memcpy(&val, &x->bytecode[i], 4);
                                printf("%d", val);
                                i += 4;
                                break;
                            }

                            case GINSTRDATATYPE_INT64:
                            {
                                int64_t val;
                                memcpy(&val, &x->bytecode[i], 8);
                                printf("%lld", val);
                                i += 8;
                                break;
                            }

                            case GINSTRDATATYPE_NUMBER32:
                            {
                                float val;
                                memcpy(&val, &x->bytecode[i], 4);

                                printf("%f", val);
                                i += 4;
                                break;
                            }

                            case GINSTRDATATYPE_NUMBER64:
                            {
                                double val;
                                memcpy(&val, &x->bytecode[i], 8);

                                printf("%lf", val);
                                i += 8;
                                break;
                            }

                            case GINSTRDATATYPE_CHAR:
                            {
                                char ch = x->bytecode[i];
                                printf("%c", ch);
                                ++i;
                                break;
                            }

                            default:
                            {
                                printf("[unknown type: %d:%zu]", immediatedatatype, i+1);
                                break;
                            }
                        }
                        printf("\n");
                        break;
                    
                    case GINSTR_JNTS:
                        printf("JNTS ");
                        ++i;
                        printf("%d\n", x->bytecode[i]);
                        ++i;
                        break;

                    case GINSTR_JNT:
                        printf("JNT ");
                        ++i;
                        G_int64 size;
                        memcpy(&size, &x->bytecode[i], sizeof(G_int64));
                        printf("%lld\n", size);
                        i += sizeof(G_int64);//sizeof(G_int64);
                        break;

                    case GINSTR_ADD:
                        printf("ADD\n");
                        ++i;
                        break;

                    case GINSTR_SUB:
                        printf("SUB\n");
                        ++i;
                        break;

                    case GINSTR_MUL:
                        printf("MUL\n");
                        ++i;
                        break;

                    case GINSTR_DIV:
                        printf("DIV\n");
                        ++i;
                        break;

                    case GINSTR_MOD:
                        printf("MOD\n");
                        ++i;
                        break;

                    case GINSTR_POW:
                        printf("POW\n");
                        ++i;
                        break;

                    case GINSTR_AND:
                        printf("AND\n");
                        ++i;
                        break;

                    case GINSTR_OR:
                        printf("OR\n");
                        ++i;
                        break;

                    case GINSTR_NOT:
                        printf("NOT\n");
                        ++i;
                        break;

                    case GINSTR_NEG:
                        printf("NEG\n");
                        ++i;
                        break;

                    case GINSTR_CALL:
                        printf("CALL\n");
                        ++i;
                        break;

                    case GINSTR_EQU:
                        printf("EQU\n");
                        ++i;
                        break;

                    case GINSTR_NOT_EQU:
                        printf("NOT-EQU\n");
                        ++i;
                        break;

                    case GINSTR_GT:
                        printf("GT\n");
                        ++i;
                        break;

                    case GINSTR_LT:
                        printf("LT\n");
                        ++i;
                        break;

                    case GINSTR_GT_EQU:
                        printf("GT-EQU\n");
                        ++i;
                        break;

                    case GINSTR_LT_EQU:
                        printf("LT-EQU\n");
                        ++i;
                        break;

                    default:
                        printf("err %d\n", x->bytecode[i]);
                        ++i;
                        break;
                }
            }

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