#ifndef IR_C
#define IR_C

#include "parser.h"
#include "ast.h"
#include "ir.h"
#include "../bytecode.h"
#include "../errors.h"
#include "../../../include/declarations.h"
#include "../../etc/log.h"
#include <math.h>
#include <string.h>
#include <inttypes.h>

#define GUIN_IR_CONVERT_FAILED   0
#define GUIN_IR_CONVERT_CONTINUE 1
#define GUIN_IR_CONVERT_FINISHED 2

static GUIN_Bytecode* GUIN_IR_CONVERT_ASTSCOPE(const GUIN_ASTScope* astscope);


GINSTR GUIN_ExpressionNodeType_to_GINSTR(GUIN_ExpressionNodeType x)
{
    switch (x)
    {
        case GUIN_EXPRNODE_ADD:          return GINSTR_ADD;
        case GUIN_EXPRNODE_SUB:          return GINSTR_SUB;
        case GUIN_EXPRNODE_MUL:          return GINSTR_MUL;
        case GUIN_EXPRNODE_DIV:          return GINSTR_DIV;
        case GUIN_EXPRNODE_MOD:          return GINSTR_MOD;
        case GUIN_EXPRNODE_POW:          return GINSTR_POW;

        case GUIN_EXPRNODE_AND:          return GINSTR_AND;
        case GUIN_EXPRNODE_OR:           return GINSTR_OR;
        case GUIN_EXPRNODE_NOT:          return GINSTR_NOT;
        case GUIN_EXPRNODE_NEG:          return GINSTR_NEG;

        case GUIN_EXPRNODE_EQU:          return GINSTR_EQU;
        case GUIN_EXPRNODE_NOT_EQU:      return GINSTR_NOT_EQU;
        case GUIN_EXPRNODE_GT:           return GINSTR_GT;
        case GUIN_EXPRNODE_LT:           return GINSTR_LT;
        case GUIN_EXPRNODE_GT_EQU:       return GINSTR_GT_EQU;
        case GUIN_EXPRNODE_LT_EQU:       return GINSTR_LT_EQU;
        default: return GINSTR_NULL;
    }
}

#define is_number_int_expr(expr) (expr == EXPRNODE_INT || expr == EXPRNODE_NUMBER)
#define is_immediate_value(expr) (is_number_int_expr(expr) || expr == EXPRNODE_BOOL || expr == EXPRNODE_CHAR || expr == EXPRNODE_STRING || expr == EXPRNODE_NIL || expr == EXPRNODE_FUNCTION_LITERAL)
// please leave arg3 as false, its just for chaining, NOT LISTENING TO THIS MESSAGE WILL MESS UP THE FUNCTION
static GUIN_ExpressionNodeType GUIN_IR_CONVERT_expression(GUIN_ExpressionNodeAST* expr, GUIN_Bytecode** addr_to_bytecode, bool dont_push_load)
{
    GUIN_ExpressionNodeType x = GUIN_EXPRNODE_UNINIT;

    if (!expr) return x;
    if (!addr_to_bytecode) return x;

    GUIN_ubyte pathways = GUIN_get_pathway_count_of_ExpressionNodeAST(expr->type);
    GUIN_Bytecode* bytecode = *addr_to_bytecode;

    switch (pathways)
    {
        case 2:
        {
            x = expr->type;
            GUIN_IR_CONVERT_expression(expr->left,  addr_to_bytecode, false);
            GUIN_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);
            GUIN_add_Bytecode_one_byte(bytecode, (GUIN_ubyte)GUIN_ExpressionNodeType_to_GINSTR(expr->type));
            break;
        }

        case 1:
        {
            x = expr->type;
            switch (x) 
            {
                case GUIN_EXPRNODE_NEG:
                {
                    GUIN_ExpressionNodeType child = expr->right->type;

                    if (child == GUIN_EXPRNODE_INT) {
                        expr->right->data.integer = -expr->right->data.integer;
                        GUIN_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);
                        break;
                    }
                    else if (child == GUIN_EXPRNODE_NUMBER) {
                        expr->right->data.number = -expr->right->data.number;
                        GUIN_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);
                        break;
                    }

                    // if it doesnt meet the requirements above, it will just use the NEG opcode
                    GUIN_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);
                    GUIN_add_Bytecode_one_byte(bytecode, (GUIN_ubyte)GINSTR_NEG);
                    break;
                }

                case GUIN_EXPRNODE_CALL:
                {
                    GUIN_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);

                    for (size_t i = 0; i < expr->data.exprFuncCallAST.expression_args_length; ++i)
                        GUIN_IR_CONVERT_expression(expr->data.exprFuncCallAST.expression_args[i]->top, addr_to_bytecode, false);

                    GUIN_add_Bytecode_one_byte(bytecode, (GUIN_ubyte)GINSTR_CALL);
                    GUIN_add_Bytecode_one_byte(bytecode, (GUIN_ubyte)expr->data.exprFuncCallAST.expression_args_length);
                    break;
                }


                case GUIN_EXPRNODE_PARENTHESIS:
                    GUIN_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);
                    break;

                default:
                    GUIN_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);
                    GUIN_add_Bytecode_one_byte(bytecode, (GUIN_ubyte)GUIN_ExpressionNodeType_to_GINSTR(expr->type));
                    break;
            }
            break;
        }

        case 0: 
        {
            x = expr->type;
            switch (expr->type)
            {
                case GUIN_EXPRNODE_STRING:
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_STRING);
                    GUIN_add_Bytecode_w_byte_size(bytecode, &expr->data.string_identifier.length, sizeof(size_t));
                    GUIN_add_Bytecode_String_no_size_embedded(bytecode, &expr->data.string_identifier);
                    break;

                case GUIN_EXPRNODE_INT:
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_INT64);
                    GUIN_add_Bytecode_w_byte_size(bytecode, &expr->data.integer, sizeof(GUIN_int64));
                    break;

                case GUIN_EXPRNODE_NUMBER:
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_NUMBER64);
                    GUIN_add_Bytecode_w_byte_size(bytecode, &expr->data.number, sizeof(GUIN_number64));
                    break;

                case GUIN_EXPRNODE_BOOL:
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_BOOL);
                    GUIN_add_Bytecode_one_byte(bytecode, (GUIN_ubyte)expr->data.bl);
                    break;

                case GUIN_EXPRNODE_CHAR:
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_CHAR);
                    GUIN_add_Bytecode_one_byte(bytecode, (GUIN_ubyte)expr->data.ch);
                    break;
                
                case GUIN_EXPRNODE_NIL:
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_NIL);
                    break;

                case GUIN_EXPRNODE_GLOBAL_IDENTIFIER:
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTR_LOAD_GLOBAL);
                    GUIN_add_Bytecode_String(bytecode, &expr->data.string_identifier);
                    if (!dont_push_load) {
                        GUIN_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_LOAD);
                    }
                    break;

                case GUIN_EXPRNODE_LOCAL_IDENTIFIER:
                    GUIN_add_Bytecode_one_byte(bytecode, GINSTR_LOAD_LOCAL);
                    GUIN_add_Bytecode_w_byte_size(bytecode, &expr->data.slot_num, sizeof(GUIN_LOCAL_SLOT_INT));
                    if (!dont_push_load) {
                        GUIN_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_LOAD);
                    }
                    break;

                default:
                    GUIN_log("expr G_IR_CONVERT_expression err %d:%s\n", expr->type, GUIN_ExpressionNodeType_to_string(expr->type));
                    x = GUIN_EXPRNODE_UNINIT;
                    GUIN_destroy_Bytecode_ptr(addr_to_bytecode);
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

static int GUIN_IR_ADD_AST_TO_BYTECODE(const GUIN_AST astnode, GUIN_Bytecode** addr_to_bytecode)
{
    if (!addr_to_bytecode) return GUIN_IR_CONVERT_FAILED;
    if (astnode.error)     return GUIN_IR_CONVERT_FAILED;

    switch (astnode.nodetype) 
    {
        case GUIN_ASTNODE_DECLARATION: {
            GUIN_ExpressionNodeType ir_tree = GUIN_IR_CONVERT_expression(astnode.declarationAST.expression->top, addr_to_bytecode, false);

            if (ir_tree == GUIN_EXPRNODE_UNINIT) {
                GUIN_log("failed!\n");
                GUIN_destroy_Bytecode_ptr(addr_to_bytecode);
                return GUIN_IR_CONVERT_FAILED;
            }

            if (astnode.declarationAST.slot == 0) {
                GUIN_add_Bytecode_one_byte(*addr_to_bytecode, (GUIN_ubyte)GINSTR_DECLARE_GLOBAL);

                // metadata for the declaration
                GUIN_add_Bytecode_one_byte(*addr_to_bytecode, (GUIN_ubyte)GUIN_ASTDatatype_to_Bytecode_Datatype(astnode.declarationAST.info.datatype));
                // str of global name
                GUIN_uint64 length = astnode.declarationAST.info.identifier.length;
                // DO NOT FORGET TO CHANGE BELOW sizeof(datatype) FOR length PLEASE
                GUIN_add_Bytecode_w_byte_size(*addr_to_bytecode, &length, sizeof(GUIN_uint64));
                GUIN_add_Bytecode_w_byte_size(*addr_to_bytecode, astnode.declarationAST.info.identifier.content, length);
            } else {
                GUIN_add_Bytecode_one_byte(*addr_to_bytecode, (GUIN_ubyte)GINSTR_DECLARE_LOCAL);

                // metadata for the declaration
                GUIN_add_Bytecode_one_byte(*addr_to_bytecode, (GUIN_ubyte)GUIN_ASTDatatype_to_Bytecode_Datatype(astnode.declarationAST.info.datatype));

                // designate the slot we plan to target
                GUIN_LOCAL_SLOT_INT slotnum = astnode.declarationAST.slot-1;
                // DO NOT FORGET TO CHANGE BELOW sizeof(datatype) FOR slotnum PLEASE
                GUIN_add_Bytecode_w_byte_size(*addr_to_bytecode, &slotnum, sizeof(GUIN_LOCAL_SLOT_INT));
            }
            break;
        }

        case GUIN_ASTNODE_IF: {
            GUIN_ExpressionNodeType ir_tree = GUIN_IR_CONVERT_expression(astnode.ifWhileAST.expression->top, addr_to_bytecode, false);
            if (ir_tree == GUIN_EXPRNODE_UNINIT) {
                GUIN_log("failed ir tree!\n");
                GUIN_destroy_Bytecode_ptr(addr_to_bytecode);
                return GUIN_IR_CONVERT_FAILED;
            }

            GUIN_Bytecode* if_conv = GUIN_IR_CONVERT_ASTSCOPE(&astnode.ifWhileAST.nodes);
            if (!if_conv) {
                GUIN_log("failed to get if conversion!\n");
                GUIN_destroy_Bytecode_ptr(addr_to_bytecode);
                return GUIN_IR_CONVERT_FAILED;
            }

            /*
            
            THE RULES FOR RUNTIME JUMP OPERATIONS:
                IT NEEDS TO FIRST READ THE ENTIRE JUMP VAL BEFORE DECIDING TO JUMP,
                THIS MEANS THAT WHEN JUMPING FORWARD, YOU *DO NOT* INCLUDE THE BYTES
                THE OPERATION HAS
            EXAMPLE IF YOU AIM TO JUMP 63 BYTES FORWARD:
                JNT 63   ;not including the 2 bytes from JNTS and 63 (0x3f)
                *NOT*
                JNT 65   ;INCLUDING the 2 bytes
            EXAMPLE FOR GOING BACKWARDS YOU *MUST INCLUDE* IF YOU AIM TO JUMP 63 BYTES BEHIND:
                JNT -65  ;including the 2 bytes from JNTS and 63 (0x3f)
                *NOT*
                JNT -63  ;NOT INCLUDING the 2 bytes 

            */

            if (if_conv->length <= 32767) {
                GUIN_add_Bytecode_one_byte(*addr_to_bytecode, (GUIN_ubyte)GINSTR_JNT);
                GUIN_int16 len = (GUIN_int16)if_conv->length;
                GUIN_add_Bytecode_w_byte_size(*addr_to_bytecode, (GUIN_ubyte*)&len, sizeof(GUIN_int16));
            } else {
                GUIN_add_Bytecode_one_byte(*addr_to_bytecode, (GUIN_ubyte)GINSTR_JNTL);
                GUIN_int64 len = (GUIN_int64)if_conv->length;
                GUIN_add_Bytecode(*addr_to_bytecode, (GUIN_ubyte*)&len, sizeof(GUIN_int64));
            }

            GUIN_add_Bytecode_w_byte_size(*addr_to_bytecode, if_conv->bytecode, if_conv->length);
            GUIN_destroy_Bytecode_ptr(&if_conv);
            break;
        }

        case GUIN_ASTNODE_WHILE: {
            GUIN_Bytecode* ir_tree_bc = GUIN_init_Bytecode_ptr();
            GUIN_ExpressionNodeType ir_tree = GUIN_IR_CONVERT_expression(astnode.ifWhileAST.expression->top, &ir_tree_bc, false);
            if (ir_tree == GUIN_EXPRNODE_UNINIT) {
                GUIN_log("failed ir tree!\n");
                GUIN_destroy_Bytecode_ptr(&ir_tree_bc);
                GUIN_destroy_Bytecode_ptr(addr_to_bytecode);
                return GUIN_IR_CONVERT_FAILED;
            }

            GUIN_Bytecode* while_conv = GUIN_IR_CONVERT_ASTSCOPE(&astnode.ifWhileAST.nodes);
            if (!while_conv) {
                GUIN_log("failed to get while conversion!\n");
                GUIN_destroy_Bytecode_ptr(&ir_tree_bc);
                GUIN_destroy_Bytecode_ptr(addr_to_bytecode);
                return GUIN_IR_CONVERT_FAILED;
            }

            GUIN_add_Bytecode_w_byte_size(*addr_to_bytecode, ir_tree_bc->bytecode, ir_tree_bc->length);
            if (while_conv->length <= 32766 - (2*(sizeof(GUIN_int16) + sizeof(GUIN_ubyte)) + ir_tree_bc->length)) {
                GUIN_add_Bytecode_one_byte(while_conv, (GUIN_ubyte)GINSTR_JMP);

                // for the backwards jump
                GUIN_int16 len = -(while_conv->length + sizeof(GUIN_ubyte) + 2*sizeof(GUIN_int16) + ir_tree_bc->length);
                GUIN_add_Bytecode_w_byte_size(while_conv, (GUIN_ubyte*)&len, sizeof(GUIN_int16));

                // we get rid of the size of the JNT + the IR tree in order to jump past the while scope properly
                len = -(len + sizeof(GUIN_ubyte) + sizeof(GUIN_int16) + ir_tree_bc->length);

                GUIN_add_Bytecode_one_byte(*addr_to_bytecode, (GUIN_ubyte)GINSTR_JNT);
                GUIN_add_Bytecode_w_byte_size(*addr_to_bytecode, (GUIN_ubyte*)&len, sizeof(GUIN_int16));
            } else {
                GUIN_add_Bytecode_one_byte(while_conv, (GUIN_ubyte)GINSTR_JMP);

                // for the backwards jump
                GUIN_int64 len = -(while_conv->length + sizeof(GUIN_ubyte) + 2*sizeof(GUIN_int64) + ir_tree_bc->length);
                GUIN_add_Bytecode_w_byte_size(while_conv, (GUIN_ubyte*)&len, sizeof(GUIN_int64));

                // we get rid of the size of the JNT + the IR tree in order to jump past the while scope properly
                len = -(len + sizeof(GUIN_ubyte) + sizeof(GUIN_int64) + ir_tree_bc->length);

                GUIN_add_Bytecode_one_byte(*addr_to_bytecode, (GUIN_ubyte)GINSTR_JNT);
                GUIN_add_Bytecode_w_byte_size(*addr_to_bytecode, (GUIN_ubyte*)&len, sizeof(GUIN_int64));
            }


            GUIN_add_Bytecode_w_byte_size(*addr_to_bytecode, while_conv->bytecode, while_conv->length);
            GUIN_destroy_Bytecode_ptr(&while_conv);
            GUIN_destroy_Bytecode_ptr(&ir_tree_bc);
            break;
        }

        case GUIN_ASTNODE_SCOPE: {
            GUIN_Bytecode* scope_conv = GUIN_IR_CONVERT_ASTSCOPE(&astnode.scopeAST);
            if (!scope_conv) {
                GUIN_log("failed to get scope conversion!\n");
                GUIN_destroy_Bytecode_ptr(&scope_conv);
                GUIN_destroy_Bytecode_ptr(addr_to_bytecode);
                return GUIN_IR_CONVERT_FAILED;
            }
            GUIN_add_Bytecode_w_byte_size(*addr_to_bytecode, scope_conv->bytecode, scope_conv->length);
            GUIN_destroy_Bytecode_ptr(&scope_conv);
            break;
        }


        case GUIN_ASTNODE_CLEAR_LOCAL_SLOT: {
            GUIN_log("CLEAR\n");
            GUIN_add_Bytecode_one_byte(*addr_to_bytecode, (GUIN_ubyte)GINSTR_CLEAR_LOCAL);
            GUIN_add_Bytecode_w_byte_size(*addr_to_bytecode, &astnode.clearLocalSlotAST.slot, sizeof(GUIN_LOCAL_SLOT_INT));
        }

        case GUIN_ASTNODE_IGNORE:
            break;

        case GUIN_ASTNODE_END:
            return GUIN_IR_CONVERT_FINISHED;

        default:
            printf("IDK HOW THE IR IS MEANT TO HANDLE THIS ONE ERROR ERROR!!!!!!!WKAJDLKAWIFJ (%d)\n", astnode.nodetype);
            exit(1);
    }

    GUIN_log("reached the end\n");
    return GUIN_IR_CONVERT_CONTINUE;
}

// assumes its in a local scope
static GUIN_Bytecode* GUIN_IR_CONVERT_ASTSCOPE(const GUIN_ASTScope* astscope)
{
    GUIN_Bytecode* bytecode = GUIN_init_Bytecode_ptr();
    if (!bytecode) return NULL;

    for (size_t i = 0; i < astscope->length; ++i) {
        GUIN_log("attempt %zu:%zu:\n", i, astscope->length);
        int code = GUIN_IR_ADD_AST_TO_BYTECODE(astscope->nodes[i], &bytecode);
        
        GUIN_log("code: %d\n", code);

        // error
        if (code == GUIN_IR_CONVERT_FAILED) {
            GUIN_destroy_Bytecode_ptr(&bytecode);
            return NULL;   
        }

        // end
        if (code == GUIN_IR_CONVERT_FINISHED) {
            break;
        }
    }

    return bytecode;
}

GUIN_Bytecode* GUIN_IR_CONVERT(GUIN_IR* ir, const bool on_global)
{
    ///////////////////////////////////////////////////////////
    ///                                                     ///
    /// SIZE_T_OF_PLATFORM GETS TEMP CHANGE PLS CHANGE BACK ///
    ///                                                     ///
    ///////////////////////////////////////////////////////////
    GUIN_ParseState pState = GUIN_init_ParseState(&ir->source);
    GUIN_Bytecode* bytecode = GUIN_init_Bytecode_ptr();
    GUIN_AST astnode = (GUIN_AST){0};

if (bytecode) {
    { // MAGIC
        if (on_global) GUIN_add_Bytecode_w_byte_size(bytecode, "$GUINEA", 7);
    }
    bool reached_the_end = false;
    bool error = false;
    while (!reached_the_end && !error) {
        GUIN_log_push_layer();
        GUIN_AST astnode = GUIN_parse_segment(&pState, GUIN_TK_EOF, true);
        GUIN_log_pop_layer();

        if (astnode.error) {
            if (pState.current.type == GUIN_TK_Identifier)
                GUIN_ERROR_std_err_message(stdout, ir->filename, ir->source.content, pState.errmsg, "", pState.current.line, pState.current.column, pState.current.string.length);
            else
                GUIN_ERROR_std_err_message(stdout, ir->filename, ir->source.content, pState.errmsg, "", pState.current.line, pState.current.column, pState.current.length);
            error = true;
            GUIN_destroy_Bytecode_ptr(&bytecode);
            break;
        }

        int code = GUIN_IR_ADD_AST_TO_BYTECODE(astnode, &bytecode);
        
        // error
        if (code == GUIN_IR_CONVERT_FAILED) {
            error = true;
        }

        // 1 == continue

        // end
        if (code == GUIN_IR_CONVERT_FINISHED) {
            reached_the_end = true;
        }

        GUIN_destroy_AST(&astnode);
    }
}
    GUIN_destroy_ParseState(&pState);
    // means it wasnt erased
    if (astnode.nodetype != GUIN_ASTNODE_IGNORE) GUIN_destroy_AST(&astnode);
    return bytecode;
}

#endif
