#include <stdio.h>
#include <stdlib.h>
#include "etc/strings.c"
#include "etc/string_manipulation.c"
#include "etc/input.c"
#include "etc/log.c"

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

int main(int argc, char** argv)
{
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

    ubyte mode = 0;

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

        G_Bytecode* x = G_IR_CONVERT(&ir, SIZE_T);

        if (x) {
            /*
            for (size_t i = 0; i < x->length; i++) {
                printf("%d ", x->bytecode[i]);
            }
            putchar(10);

            freopen("file.gs", "w", stdout);

            for (size_t i = 7; i < x->length;) {
                switch (x->bytecode[i])
                {
                    case GINSTR_DECLARE_GLOBAL:
                        ++i;
                        GINSTR_Datatype globaldatatype = x->bytecode[i];

                        printf("declare global [%s] ", G_Bytecode_Datatype_to_str(globaldatatype));

                        ++i;

                        String* name = get_string_in_G_Bytecode(x, i, 8);
                        if (!name) {
                            printf("no name\n");
                            return 1;
                        }

                        i += 8 + name->length;
                        printf("%ld: ", name->length);
                        fwrite(name->content, sizeof(ubyte), name->length, stdout);
                        printf("\n");

                        switch (datatype)
                        {
                            case GINSTRDATATYPE_STRING:
                                break;
                            case GINSTRDATATYPE_INT64:
                                int64_t val =
                        }
                        clearstring_ptr(&name);
                        break;

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
                                printf("nil");
                                ++i;
                                break;
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
                                printf("%ld", val);
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
                                printf("[unknown type: %d]\n", immediatedatatype);
                                break;
                            }
                        }
                        printf("\n");
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
                        break;
                }
            }
        */

            FILE* tosave = fopen("file.gbc", "w");
            if (tosave) {
                fwrite(x->bytecode, sizeof(ubyte), x->length, tosave);
                fclose(tosave);
            }
        }

        //FILE* save = fopen("file.gbc", "w");
        //if (save) fwrite(x->bytecode, sizeof(ubyte), x->length, save);

        destroy_G_Bytecode_ptr(&x);
        clearstring(&inp);
    }

    return 0;
}