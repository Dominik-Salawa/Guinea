#ifndef IR_C
#define IR_C

#include "parser.h"
#include "ast.h"
#include "../bytecode.h"
#include "../errors.h"
#include "../../etc/declarations.h"
#include "../../etc/log.h"
#include <math.h>
#include <string.h>
#include <inttypes.h>

#ifndef SIZE_T
#define SIZE_T 8
#endif

typedef struct G_IR {
    char* filename;
    ubyte ver[2];
    String source;
} G_IR;

GINSTR ExpressionNodeType_to_GINSTR(ExpressionNodeType x)
{
    switch (x)
    {
        case EXPRNODE_ADD:          return GINSTR_ADD;
        case EXPRNODE_SUB:          return GINSTR_SUB;
        case EXPRNODE_MUL:          return GINSTR_MUL;
        case EXPRNODE_DIV:          return GINSTR_DIV;
        case EXPRNODE_MOD:          return GINSTR_MOD;
        case EXPRNODE_POW:          return GINSTR_POW;

        case EXPRNODE_AND:          return GINSTR_AND;
        case EXPRNODE_NOT:          return GINSTR_NOT;
        case EXPRNODE_NEG:          return GINSTR_NEG;

        case EXPRNODE_EQU:          return GINSTR_EQU;
        case EXPRNODE_NOT_EQU:      return GINSTR_NOT_EQU;
        case EXPRNODE_GT:           return GINSTR_GT;
        case EXPRNODE_LT:           return GINSTR_LT;
        case EXPRNODE_GT_EQU:       return GINSTR_GT_EQU;
        case EXPRNODE_LT_EQU:       return GINSTR_LT_EQU;
        default: return GINSTR_NULL;
    }
}

#define is_number_int_expr(expr) (expr == EXPRNODE_INT || expr == EXPRNODE_NUMBER)
#define is_immediate_value(expr) (is_number_int_expr(expr) || expr == EXPRNODE_BOOL || expr == EXPRNODE_CHAR || expr == EXPRNODE_STRING || expr == EXPRNODE_NIL || expr == EXPRNODE_FUNCTION_LITERAL)
// please leave arg2 as just NULL, its just for chaining
static ExpressionNodeType G_IR_CONVERT_expression(ExpressionNodeAST* expr, G_Bytecode** addr_to_bytecode)
{
    ExpressionNodeType x = EXPRNODE_UNINIT;

    if (!expr) return x;
    if (!addr_to_bytecode) return x;

    ubyte pathways = 0;
    switch (expr->type)
    {
        case EXPRNODE_ADD: pathways = 2; break;
        case EXPRNODE_SUB: pathways = 2; break;
        case EXPRNODE_MUL: pathways = 2; break;
        case EXPRNODE_DIV: pathways = 2; break;
        case EXPRNODE_MOD: pathways = 2; break;
        case EXPRNODE_POW: pathways = 2; break;

        case EXPRNODE_EQU: pathways = 2; break;
        case EXPRNODE_NOT_EQU: pathways = 2; break;

        case EXPRNODE_GT: pathways = 2; break;
        case EXPRNODE_LT: pathways = 2; break;
        case EXPRNODE_GT_EQU: pathways = 2; break;
        case EXPRNODE_LT_EQU: pathways = 2; break;

        case EXPRNODE_NEG: pathways = 1; break;
        case EXPRNODE_NOT: pathways = 1; break;
        case EXPRNODE_CALL: pathways = 1; break;
        default: pathways = 0;
    }

    G_log("pathway count: %d\n", pathways);

    G_Bytecode* bytecode = *addr_to_bytecode;

    switch (pathways)
    {
        case 2:
        {
            x = expr->type;
            G_log("2 path\n");
            G_IR_CONVERT_expression(expr->left,  addr_to_bytecode);
            G_IR_CONVERT_expression(expr->right, addr_to_bytecode);
            add_G_Bytecode_one_byte(bytecode, (ubyte)ExpressionNodeType_to_GINSTR(expr->type));
            break;
        }

        case 1:
        {
            x = expr->type;
            G_log("1 path\n");
            G_IR_CONVERT_expression(expr->right, addr_to_bytecode);
            add_G_Bytecode_one_byte(bytecode, (ubyte)ExpressionNodeType_to_GINSTR(expr->type));
            break;
        }

        case 0: 
        {
            G_log("0 path\n");
            x = expr->type;
            switch (expr->type)
            {
                case EXPRNODE_STRING:
                    G_log("string\n");
                    add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    add_G_Bytecode_one_byte(bytecode, (ubyte)GINSTRDATATYPE_STRING);
                    add_G_Bytecode_w_byte_size(bytecode, &expr->data.string_identifier.length, SIZE_T);
                    add_G_Bytecode_String_no_size_embedded(bytecode, &expr->data.string_identifier);
                    break;

                case EXPRNODE_INT:
                    G_log("int\n");
                    add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    add_G_Bytecode_one_byte(bytecode, (ubyte)GINSTRDATATYPE_INT64);
                    add_G_Bytecode_w_byte_size(bytecode, &expr->data.integer, sizeof(int64_t));
                    break;

                case EXPRNODE_NUMBER:
                    G_log("number\n");
                    add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    add_G_Bytecode_one_byte(bytecode, (ubyte)GINSTRDATATYPE_NUMBER64);
                    add_G_Bytecode_w_byte_size(bytecode, &expr->data.number, sizeof(double));
                    break;

                case EXPRNODE_BOOL:
                    G_log("bool\n");
                    add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    add_G_Bytecode_one_byte(bytecode, (ubyte)GINSTRDATATYPE_BOOL);
                    add_G_Bytecode_one_byte(bytecode, (ubyte)expr->data.bl);
                    break;

                case EXPRNODE_CHAR:
                    G_log("char\n");
                    add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    add_G_Bytecode_one_byte(bytecode, (ubyte)GINSTRDATATYPE_CHAR);
                    add_G_Bytecode_one_byte(bytecode, (ubyte)expr->data.ch);
                    break;
                
                case EXPRNODE_NIL:
                    G_log("nil\n");
                    add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    add_G_Bytecode_one_byte(bytecode, (ubyte)GINSTRDATATYPE_NIL);
                    break;

                case EXPRNODE_IDENTIFIER:
                    G_log("identifier\n");
                    printf("\n\nFOR NOW VARIABLES IN ASSIGNMENT ARE DISABLED AS I NEED TO CONFIG NAME STUFF!\n\n");
                    //add_G_Bytecode_one_byte(bytecode, GINSTR)
                    break;

                default:
                    printf("expr G_IR_CONVERT_expression err %d:%s\n", expr->type, ExpressionNodeType_to_string(expr->type));
                    x = EXPRNODE_UNINIT;
                    destroy_G_Bytecode_ptr(addr_to_bytecode);
                    return x;

            }
            break;
        }

        default:
            printf("err IR CONV RESULT\n");
            exit(1);
    }
    return x;
}

static bool on_global_ir_convert = true;
G_Bytecode* G_IR_CONVERT(G_IR* ir, ubyte SIZE_T_OF_PLATFORM)
{
    ///////////////////////////////////////////////////////////
    ///                                                     ///
    /// SIZE_T_OF_PLATFORM GETS TEMP CHANGE PLS CHANGE BACK ///
    ///                                                     ///
    ///////////////////////////////////////////////////////////
    SIZE_T_OF_PLATFORM = 8;

    ParseState pState = init_ParseState(&ir->source);
    G_Bytecode* bytecode = init_G_Bytecode_ptr();
    G_AST astnode;

if (bytecode) {
    { // MAGIC
        if (on_global_ir_convert) add_G_Bytecode_w_byte_size(bytecode, "$GUINEA", 7);
    }
    bool reached_the_end = false;
    bool error = false;
    while (!reached_the_end && !error) {
        G_log_push_layer();
        G_AST astnode = parse_segment(&pState, TK_EOF, true);
        G_log_pop_layer();

        if (astnode.error) {
            if (pState.current.type == TK_Identifier)
                std_err_message(stdout, ir->filename, ir->source.content, pState.errmsg, pState.current.line, pState.current.column, pState.current.string.length);
            else
                std_err_message(stdout, ir->filename, ir->source.content, pState.errmsg, pState.current.line, pState.current.column, pState.current.length);
            error = true;
            destroy_G_Bytecode_ptr(&bytecode);
            break;
        }

        switch (astnode.nodetype) 
        {
            case ASTNODE_DECLARATION:
                ExpressionNodeType ir_tree = G_IR_CONVERT_expression(astnode.declarationAST.expression->top, &bytecode);

                if (ir_tree == EXPRNODE_UNINIT) {
                    G_log("failed!\n");
                    error = true;
                    if (!pState.errmsg)
                        pState.errmsg = "Failed to convert Declaration Expression!";
                    destroy_G_Bytecode_ptr(&bytecode);
                    break;
                }

                add_G_Bytecode_one_byte(bytecode, (ubyte)GINSTR_DECLARE_GLOBAL);

                // metadata for the declaration
                add_G_Bytecode_one_byte(bytecode, (ubyte)ASTDatatype_to_G_Bytecode_Datatype(astnode.declarationAST.info.datatype));
                // str of global name
                add_G_Bytecode_w_byte_size(bytecode, &astnode.declarationAST.info.identifier.length, SIZE_T_OF_PLATFORM);
                add_G_Bytecode(bytecode, (ubyte*)astnode.declarationAST.info.identifier.content, astnode.declarationAST.info.identifier.length);
                
                //G_log_ExpressionNodeAST(astnode.declarationAST.expression->top);
                //G_log("\n");

                G_log("exprnodetype: %s\n", ExpressionNodeType_to_string(astnode.declarationAST.expression->top->type));
                break;

            case ASTNODE_IGNORE:
                break;

            case ASTNODE_END:
                reached_the_end = true;
                break;

            default:
                printf("IDK HOW THE IR IS MEANT TO HANDLE THIS ONE ERROR ERROR!!!!!!!WKAJDLKAWIFJ (%d)\n", astnode.nodetype);
                exit(1);
        }

        destroy_G_AST(&astnode);
    }
}
    destroy_ParseState(&pState);
    destroy_G_AST(&astnode);
    return bytecode;
}

#endif