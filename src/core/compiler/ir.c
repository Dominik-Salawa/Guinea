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

#define guin_IR_CONVERT_FAILED   0
#define guin_IR_CONVERT_CONTINUE 1
#define guin_IR_CONVERT_FINISHED 2

static guin_Bytecode* guin_IR_CONVERT_ASTSCOPE(const guin_ASTScope* astscope);


GINSTR guin_ExpressionNodeType_to_GINSTR(guin_ExpressionNodeType x)
{
    switch (x)
    {
        case guin_EXPRNODE_ADD:          return GINSTR_ADD;
        case guin_EXPRNODE_SUB:          return GINSTR_SUB;
        case guin_EXPRNODE_MUL:          return GINSTR_MUL;
        case guin_EXPRNODE_DIV:          return GINSTR_DIV;
        case guin_EXPRNODE_MOD:          return GINSTR_MOD;
        case guin_EXPRNODE_POW:          return GINSTR_POW;

        case guin_EXPRNODE_AND:          return GINSTR_AND;
        case guin_EXPRNODE_OR:           return GINSTR_OR;
        case guin_EXPRNODE_NOT:          return GINSTR_NOT;
        case guin_EXPRNODE_NEG:          return GINSTR_NEG;

        case guin_EXPRNODE_EQU:          return GINSTR_EQU;
        case guin_EXPRNODE_NOT_EQU:      return GINSTR_NOT_EQU;
        case guin_EXPRNODE_GT:           return GINSTR_GT;
        case guin_EXPRNODE_LT:           return GINSTR_LT;
        case guin_EXPRNODE_GT_EQU:       return GINSTR_GT_EQU;
        case guin_EXPRNODE_LT_EQU:       return GINSTR_LT_EQU;
        default: return GINSTR_NULL;
    }
}

#define is_number_int_expr(expr) (expr == EXPRNODE_INT || expr == EXPRNODE_NUMBER)
#define is_immediate_value(expr) (is_number_int_expr(expr) || expr == EXPRNODE_BOOL || expr == EXPRNODE_CHAR || expr == EXPRNODE_STRING || expr == EXPRNODE_NIL || expr == EXPRNODE_FUNCTION_LITERAL)
// please leave arg3 as false, its just for chaining, NOT LISTENING TO THIS MESSAGE WILL MESS UP THE FUNCTION
static guin_ExpressionNodeType guin_IR_CONVERT_expression(guin_ExpressionNodeAST* expr, guin_Bytecode** addr_to_bytecode, bool dont_push_load, bool assign_trust_push)
{
    guin_ExpressionNodeType x = guin_EXPRNODE_UNINIT;

    if (!expr) return x;
    if (!addr_to_bytecode) return x;

    guin_ubyte pathways = guin_get_pathway_count_of_ExpressionNodeAST(expr->type);
    guin_Bytecode* bytecode = *addr_to_bytecode;

    switch (pathways)
    {
        case 2:
        {
            x = expr->type;
            guin_IR_CONVERT_expression(expr->left,  addr_to_bytecode, false, true);
            guin_IR_CONVERT_expression(expr->right, addr_to_bytecode, false, true);
            guin_add_Bytecode_one_byte(bytecode, (guin_ubyte)guin_ExpressionNodeType_to_GINSTR(expr->type));
            break;
        }

        case 1:
        {
            x = expr->type;
            switch (x) 
            {
                case guin_EXPRNODE_NEG:
                {
                    guin_ExpressionNodeType child = expr->right->type;

                    if (child == guin_EXPRNODE_INT) {
                        expr->right->data.integer = -expr->right->data.integer;
                        guin_IR_CONVERT_expression(expr->right, addr_to_bytecode, false, true);
                        break;
                    }
                    else if (child == guin_EXPRNODE_NUMBER) {
                        expr->right->data.number = -expr->right->data.number;
                        guin_IR_CONVERT_expression(expr->right, addr_to_bytecode, false, true);
                        break;
                    }

                    // if it doesnt meet the requirements above, it will just use the NEG opcode
                    guin_IR_CONVERT_expression(expr->right, addr_to_bytecode, false, true);
                    guin_add_Bytecode_one_byte(bytecode, (guin_ubyte)GINSTR_NEG);
                    break;
                }

                case guin_EXPRNODE_CALL:
                {
                    guin_IR_CONVERT_expression(expr->right, addr_to_bytecode, false, true);

                    for (size_t i = 0; i < expr->data.exprFuncCallAST.expression_args_length; ++i)
                        guin_IR_CONVERT_expression(expr->data.exprFuncCallAST.expression_args[i]->top, addr_to_bytecode, false, true);

                    guin_add_Bytecode_one_byte(bytecode, (guin_ubyte)GINSTR_CALL);
                    guin_add_Bytecode_one_byte(bytecode, (guin_ubyte)expr->data.exprFuncCallAST.expression_args_length);
                    break;
                }


                case guin_EXPRNODE_PARENTHESIS:
                    guin_IR_CONVERT_expression(expr->right, addr_to_bytecode, false, true);
                    break;

                default:
                    guin_IR_CONVERT_expression(expr->right, addr_to_bytecode, false, true);
                    guin_add_Bytecode_one_byte(bytecode, (guin_ubyte)guin_ExpressionNodeType_to_GINSTR(expr->type));
                    break;
            }
            break;
        }

        case 0: 
        {
            x = expr->type;
            switch (expr->type)
            {
                case guin_EXPRNODE_FUNCTION_LITERAL:
                    guin_log("converting function literal\n");
                    guin_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    guin_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_FUNCTION);
                    guin_add_Bytecode_one_byte(bytecode, (guin_ubyte)guin_ASTDatatype_to_Bytecode_Datatype(expr->data.function.return_type));
                    guin_Bytecode* funcbc = guin_IR_CONVERT_ASTSCOPE(&expr->data.function.scope);
                    if (!funcbc) {
                        guin_destroy_Bytecode_ptr(addr_to_bytecode);
                        return guin_EXPRNODE_UNINIT;
                    }

                    if (funcbc->length > 0 && funcbc->bytecode[funcbc->length-1] != GINSTR_RET) {
                        guin_log("add RET since its missing from func bytecode\n");
                        if (!guin_add_Bytecode_one_byte(funcbc, GINSTR_RET)) {
                            guin_destroy_Bytecode_ptr(addr_to_bytecode);
                            guin_destroy_Bytecode_ptr(&funcbc);
                            return guin_EXPRNODE_UNINIT;
                        }
                    }

                    guin_log("done add %ld\n", funcbc->length);

                    guin_add_Bytecode_w_byte_size(bytecode, &funcbc->length, sizeof(size_t));
                    guin_add_Bytecode(bytecode, funcbc->bytecode, funcbc->length);
                    guin_destroy_Bytecode_ptr(&funcbc);
                    break;

                case guin_EXPRNODE_STRING:
                    guin_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    guin_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_STRING);
                    guin_add_Bytecode_w_byte_size(bytecode, &expr->data.string_identifier.length, sizeof(size_t));
                    guin_add_Bytecode_String_no_size_embedded(bytecode, &expr->data.string_identifier);
                    break;

                case guin_EXPRNODE_INT:
                    guin_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    guin_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_INT64);
                    guin_add_Bytecode_w_byte_size(bytecode, &expr->data.integer, sizeof(guin_int64));
                    break;

                case guin_EXPRNODE_NUMBER:
                    guin_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    guin_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_NUMBER64);
                    guin_add_Bytecode_w_byte_size(bytecode, &expr->data.number, sizeof(guin_number64));
                    break;

                case guin_EXPRNODE_BOOL:
                    guin_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    guin_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_BOOL);
                    guin_add_Bytecode_one_byte(bytecode, (guin_ubyte)expr->data.bl);
                    break;

                case guin_EXPRNODE_CHAR:
                    guin_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    guin_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_CHAR);
                    guin_add_Bytecode_one_byte(bytecode, (guin_ubyte)expr->data.ch);
                    break;
                
                case guin_EXPRNODE_NIL:
                    guin_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    guin_add_Bytecode_one_byte(bytecode, GINSTRDATATYPE_NIL);
                    break;

                case guin_EXPRNODE_GLOBAL_IDENTIFIER:
                    guin_add_Bytecode_one_byte(bytecode, GINSTR_LOAD_GLOBAL);
                    guin_add_Bytecode_String(bytecode, &expr->data.string_identifier);
                    if (!dont_push_load) {
                        guin_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_LOAD);
                    }
                    break;

                case guin_EXPRNODE_LOCAL_IDENTIFIER:
                    guin_add_Bytecode_one_byte(bytecode, GINSTR_LOAD_LOCAL);
                    guin_add_Bytecode_w_byte_size(bytecode, &expr->data.slot_num, sizeof(guin_LOCAL_SLOT_INT));
                    if (!dont_push_load) {
                        guin_add_Bytecode_one_byte(bytecode, GINSTR_PUSH_LOAD);
                    }
                    break;

                default:
                    guin_log("expr G_IR_CONVERT_expression err %d:%s\n", expr->type, guin_ExpressionNodeType_to_string(expr->type));
                    x = guin_EXPRNODE_UNINIT;
                    guin_destroy_Bytecode_ptr(addr_to_bytecode);
                    return x;

            }
            break;
        }

        default:
            printf("err IR CONV RESULT\n");
            break;
    }
    return x;
}

static guin_STATUS guin_IR_ADD_AST_TO_BYTECODE(const guin_AST* astnode, guin_Bytecode** addr_to_bytecode)
{
    if (!addr_to_bytecode || !astnode) return guin_FAIL;

    switch (astnode->nodetype) 
    {
        case guin_ASTNODE_DECLARATION: {
            guin_ExpressionNodeType ir_tree = guin_IR_CONVERT_expression(astnode->declarationAST.expression->top, addr_to_bytecode, false, false);

            if (ir_tree == guin_EXPRNODE_UNINIT) {
                guin_log("failed!\n");
                guin_destroy_Bytecode_ptr(addr_to_bytecode);
                return guin_FAIL;
            }

            if (astnode->declarationAST.slot == 0) {
                guin_add_Bytecode_one_byte(*addr_to_bytecode, (guin_ubyte)GINSTR_DECLARE_GLOBAL);

                // metadata for the declaration
                guin_add_Bytecode_one_byte(*addr_to_bytecode, (guin_ubyte)guin_ASTDatatype_to_Bytecode_Datatype(astnode->declarationAST.info.datatype));
                // str of global name
                guin_uint64 length = astnode->declarationAST.info.identifier.length;
                // DO NOT FORGET TO CHANGE BELOW sizeof(datatype) FOR length PLEASE
                guin_add_Bytecode_w_byte_size(*addr_to_bytecode, &length, sizeof(guin_uint64));
                guin_add_Bytecode_w_byte_size(*addr_to_bytecode, astnode->declarationAST.info.identifier.content, length);
            } else {
                guin_add_Bytecode_one_byte(*addr_to_bytecode, (guin_ubyte)GINSTR_DECLARE_LOCAL);

                // metadata for the declaration
                guin_add_Bytecode_one_byte(*addr_to_bytecode, (guin_ubyte)guin_ASTDatatype_to_Bytecode_Datatype(astnode->declarationAST.info.datatype));

                // designate the slot we plan to target
                guin_LOCAL_SLOT_INT slotnum = astnode->declarationAST.slot-1;
                // DO NOT FORGET TO CHANGE BELOW sizeof(datatype) FOR slotnum PLEASE
                guin_add_Bytecode_w_byte_size(*addr_to_bytecode, &slotnum, sizeof(guin_LOCAL_SLOT_INT));
            }
            break;
        }

        case guin_ASTNODE_IF: {
            guin_ExpressionNodeType ir_tree = guin_IR_CONVERT_expression(astnode->ifWhileAST.expression->top, addr_to_bytecode, false, true);
            if (ir_tree == guin_EXPRNODE_UNINIT) {
                guin_log("failed ir tree!\n");
                guin_destroy_Bytecode_ptr(addr_to_bytecode);
                return guin_MEM_FAIL;
            }

            guin_Bytecode* if_conv = guin_IR_CONVERT_ASTSCOPE(&astnode->ifWhileAST.nodes);
            if (!if_conv) {
                guin_log("failed to get if conversion!\n");
                guin_destroy_Bytecode_ptr(addr_to_bytecode);
                return guin_MEM_FAIL;
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
                guin_add_Bytecode_one_byte(*addr_to_bytecode, (guin_ubyte)GINSTR_JNT);
                guin_int16 len = (guin_int16)if_conv->length;
                guin_add_Bytecode_w_byte_size(*addr_to_bytecode, (guin_ubyte*)&len, sizeof(guin_int16));
            } else {
                guin_add_Bytecode_one_byte(*addr_to_bytecode, (guin_ubyte)GINSTR_JNTL);
                guin_int64 len = (guin_int64)if_conv->length;
                guin_add_Bytecode(*addr_to_bytecode, (guin_ubyte*)&len, sizeof(guin_int64));
            }

            guin_add_Bytecode_w_byte_size(*addr_to_bytecode, if_conv->bytecode, if_conv->length);
            guin_destroy_Bytecode_ptr(&if_conv);
            break;
        }

        case guin_ASTNODE_WHILE: {
            guin_Bytecode* ir_tree_bc = guin_init_Bytecode_ptr();
            if (!ir_tree_bc)
                return guin_MEM_FAIL;

            guin_ExpressionNodeType ir_tree = guin_IR_CONVERT_expression(astnode->ifWhileAST.expression->top, &ir_tree_bc, false, true);
            if (ir_tree == guin_EXPRNODE_UNINIT) {
                guin_log("failed ir tree!\n");
                guin_destroy_Bytecode_ptr(&ir_tree_bc);
                guin_destroy_Bytecode_ptr(addr_to_bytecode);
                return guin_FAIL;
            }

            guin_Bytecode* while_conv = guin_IR_CONVERT_ASTSCOPE(&astnode->ifWhileAST.nodes);
            if (!while_conv) {
                guin_log("failed to get while conversion!\n");
                guin_destroy_Bytecode_ptr(&ir_tree_bc);
                guin_destroy_Bytecode_ptr(addr_to_bytecode);
                return guin_FAIL;
            }

            guin_add_Bytecode_w_byte_size(*addr_to_bytecode, ir_tree_bc->bytecode, ir_tree_bc->length);
            if (while_conv->length <= 32766 - (2*(sizeof(guin_int16) + sizeof(guin_ubyte)) + ir_tree_bc->length)) {
                guin_add_Bytecode_one_byte(while_conv, (guin_ubyte)GINSTR_JMP);

                // for the backwards jump
                guin_int16 len = -(while_conv->length + sizeof(guin_ubyte) + 2*sizeof(guin_int16) + ir_tree_bc->length);
                guin_add_Bytecode_w_byte_size(while_conv, (guin_ubyte*)&len, sizeof(guin_int16));

                // we get rid of the size of the JNT + the IR tree in order to jump past the while scope properly
                len = -(len + sizeof(guin_ubyte) + sizeof(guin_int16) + ir_tree_bc->length);

                guin_add_Bytecode_one_byte(*addr_to_bytecode, (guin_ubyte)GINSTR_JNT);
                guin_add_Bytecode_w_byte_size(*addr_to_bytecode, (guin_ubyte*)&len, sizeof(guin_int16));
            } else {
                guin_add_Bytecode_one_byte(while_conv, (guin_ubyte)GINSTR_JMP);

                // for the backwards jump
                guin_int64 len = -(while_conv->length + sizeof(guin_ubyte) + 2*sizeof(guin_int64) + ir_tree_bc->length);
                guin_add_Bytecode_w_byte_size(while_conv, (guin_ubyte*)&len, sizeof(guin_int64));

                // we get rid of the size of the JNT + the IR tree in order to jump past the while scope properly
                len = -(len + sizeof(guin_ubyte) + sizeof(guin_int64) + ir_tree_bc->length);

                guin_add_Bytecode_one_byte(*addr_to_bytecode, (guin_ubyte)GINSTR_JNT);
                guin_add_Bytecode_w_byte_size(*addr_to_bytecode, (guin_ubyte*)&len, sizeof(guin_int64));
            }


            guin_add_Bytecode_w_byte_size(*addr_to_bytecode, while_conv->bytecode, while_conv->length);
            guin_destroy_Bytecode_ptr(&while_conv);
            guin_destroy_Bytecode_ptr(&ir_tree_bc);
            break;
        }

        case guin_ASTNODE_SCOPE: {
            guin_Bytecode* scope_conv = guin_IR_CONVERT_ASTSCOPE(&astnode->scopeAST);
            if (!scope_conv) {
                guin_log("failed to get scope conversion!\n");
                guin_destroy_Bytecode_ptr(&scope_conv);
                guin_destroy_Bytecode_ptr(addr_to_bytecode);
                return guin_FAIL;
            }
            guin_add_Bytecode_w_byte_size(*addr_to_bytecode, scope_conv->bytecode, scope_conv->length);
            guin_destroy_Bytecode_ptr(&scope_conv);
            break;
        }


        case guin_ASTNODE_CLEAR_LOCAL_SLOT: {
            guin_log("CLEAR\n");
            guin_add_Bytecode_one_byte(*addr_to_bytecode, (guin_ubyte)GINSTR_CLEAR_LOCAL);
            guin_add_Bytecode_w_byte_size(*addr_to_bytecode, &astnode->clearLocalSlotAST.slot, sizeof(guin_LOCAL_SLOT_INT));
            break;
        }

        case guin_ASTNODE_RETURN: {
            guin_log("return\n");
            guin_IR_CONVERT_expression(astnode->returnAST.expression, addr_to_bytecode, false, true);
            guin_add_Bytecode_one_byte(*addr_to_bytecode, (guin_ubyte)GINSTR_RET);
            break;
        }

        case guin_ASTNODE_NULL: 
            return guin_FAIL;

        case guin_ASTNODE_BREAK: 
            return guin_FAIL;

        case guin_ASTNODE_CONTINUE: 
            return guin_FAIL;

        case guin_ASTNODE_IGNORE:
            break;

        case guin_ASTNODE_END:
            return guin_FINISHED;
    }

    guin_log("reached the end\n");
    return guin_CONTINUE;
}

struct guin_IR_CONVERT_ASTSCOPE_result {
    guin_STATUS status;
    guin_Bytecode* bytecode;
};

// assumes its in a local scope
static guin_Bytecode* guin_IR_CONVERT_ASTSCOPE(const guin_ASTScope* astscope)
{
    guin_Bytecode* bytecode = guin_init_Bytecode_ptr();
    if (!bytecode) return NULL;

    guin_log("length of ASTScope for guin_IR_CONVERT_ASTSCOPE: %zu\n", astscope->length);
    for (size_t i = 0; i < astscope->length; ++i) {
        guin_STATUS code = guin_IR_ADD_AST_TO_BYTECODE(&astscope->nodes[i], &bytecode);

        if (code == guin_MEM_FAIL) {
            guin_destroy_Bytecode_ptr(&bytecode);
        }

        // error
        if (code == guin_FAIL) {
            guin_destroy_Bytecode_ptr(&bytecode);
            return NULL;   
        }

        // end
        if (code == guin_FINISHED) {
            break;
        }
    }

    return bytecode;
}

guin_Bytecode* guin_IR_CONVERT(guin_IR* ir, const bool on_global)
{
    guin_ParseState pState = guin_init_ParseState(&ir->source);
    guin_Bytecode* bytecode = guin_init_Bytecode_ptr();
    guin_AST* astnode = NULL;

    bool reached_the_end = false;
    bool error = false;
    while (!reached_the_end && !error) {
        guin_log_push_layer();
        astnode = guin_parse_segment(&pState, guin_TK_EOF, true);
        guin_log_pop_layer();

        if (!astnode) {
            guin_destroy_Bytecode_ptr(&bytecode);
            guin_destroy_ParseState(&pState);
            return NULL;
        }

        if (astnode->error) {
            if (pState.current.type == guin_TK_Identifier)
                guin_ERROR_std_err_message(stdout, ir->filename, ir->source.content, pState.errmsg, "", pState.current.line, pState.current.column, pState.current.string.length);
            else
                guin_ERROR_std_err_message(stdout, ir->filename, ir->source.content, pState.errmsg, "", pState.current.line, pState.current.column, pState.current.length);
            error = true;
            guin_destroy_Bytecode_ptr(&bytecode);
            break;
        }

        guin_STATUS code = guin_IR_ADD_AST_TO_BYTECODE(astnode, &bytecode);

        // error
        if (code == guin_MEM_FAIL) {
            guin_destroy_Bytecode_ptr(&bytecode);
            guin_destroy_ParseState(&pState);
            guin_destroy_AST_ptr(&astnode);
            return NULL;
        }
        else if (code == guin_FAIL) {
            error = true;
        }
        // end
        else if (code == guin_FINISHED) {
            reached_the_end = true;
        }

        guin_destroy_AST_ptr(&astnode);
    }
    guin_destroy_ParseState(&pState);
    
    if (!guin_add_Bytecode_one_byte(bytecode, GINSTR_RET))
        guin_destroy_Bytecode_ptr(&bytecode); // so we return NULL, since adding it failed
        
    if (astnode) guin_destroy_AST_ptr(&astnode);
    return bytecode;
}

#endif
