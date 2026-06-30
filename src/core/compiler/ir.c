#ifndef IR_C
#define IR_C

#include "parser.h"
#include "ast.h"
#include "ir.h"
#include "../bytecode.h"
#include "../errors.h"
#include "../../etc/declarations.h"
#include "../../etc/log.h"
#include <math.h>
#include <string.h>
#include <inttypes.h>

#define IR_CONVERT_FAILED   0
#define IR_CONVERT_CONTINUE 1
#define IR_CONVERT_FINISHED 2

static G_Bytecode* G_IR_CONVERT_ASTSCOPE(const ASTScope* astscope);


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
        case EXPRNODE_OR:           return GINSTR_OR;
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
// please leave arg3 as false, its just for chaining, NOT LISTENING TO THIS MESSAGE WILL MESS UP THE FUNCTION
static ExpressionNodeType G_IR_CONVERT_expression(ExpressionNodeAST* expr, G_Bytecode** addr_to_bytecode, bool dont_push_load)
{
    ExpressionNodeType x = EXPRNODE_UNINIT;

    if (!expr) return x;
    if (!addr_to_bytecode) return x;

    G_ubyte pathways = get_pathway_count_of_ExpressionNodeAST(expr->type);
    G_Bytecode* bytecode = *addr_to_bytecode;

    switch (pathways)
    {
        case 2:
        {
            x = expr->type;
            G_IR_CONVERT_expression(expr->left,  addr_to_bytecode, false);
            G_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);
            add_G_Bytecode_one_byte(bytecode, (G_ubyte)ExpressionNodeType_to_GINSTR(expr->type));
            break;
        }

        case 1:
        {
            x = expr->type;
            switch (x) 
            {
                case EXPRNODE_NEG:
                {
                    ExpressionNodeType child = expr->right->type;

                    if (child == EXPRNODE_INT) {
                        expr->right->data.integer = -expr->right->data.integer;
                        G_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);
                        break;
                    }
                    else if (child == EXPRNODE_NUMBER) {
                        expr->right->data.number = -expr->right->data.number;
                        G_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);
                        break;
                    }

                    // if it doesnt meet the requirements above, it is expected to do the same as default
                    G_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);
                    add_G_Bytecode_one_byte(bytecode, (G_ubyte)GINSTR_NEG);
                    break;
                }

                case EXPRNODE_PARENTHESIS:
                    G_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);
                    break;

                default:
                    G_IR_CONVERT_expression(expr->right, addr_to_bytecode, false);
                    add_G_Bytecode_one_byte(bytecode, (G_ubyte)ExpressionNodeType_to_GINSTR(expr->type));
                    break;
            }
            break;
        }

        case 0: 
        {
            x = expr->type;
            switch (expr->type)
            {
                case EXPRNODE_STRING:
                    add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    add_G_Bytecode_one_byte(bytecode, GINSTRDATATYPE_STRING);
                    add_G_Bytecode_w_byte_size(bytecode, &expr->data.string_identifier.length, sizeof(size_t));
                    add_G_Bytecode_String_no_size_embedded(bytecode, &expr->data.string_identifier);
                    break;

                case EXPRNODE_INT:
                    add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    add_G_Bytecode_one_byte(bytecode, GINSTRDATATYPE_INT64);
                    add_G_Bytecode_w_byte_size(bytecode, &expr->data.integer, sizeof(G_int64));
                    break;

                case EXPRNODE_NUMBER:
                    add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    add_G_Bytecode_one_byte(bytecode, GINSTRDATATYPE_NUMBER64);
                    add_G_Bytecode_w_byte_size(bytecode, &expr->data.number, sizeof(G_number64));
                    break;

                case EXPRNODE_BOOL:
                    add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    add_G_Bytecode_one_byte(bytecode, GINSTRDATATYPE_BOOL);
                    add_G_Bytecode_one_byte(bytecode, (G_ubyte)expr->data.bl);
                    break;

                case EXPRNODE_CHAR:
                    add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    add_G_Bytecode_one_byte(bytecode, GINSTRDATATYPE_CHAR);
                    add_G_Bytecode_one_byte(bytecode, (G_ubyte)expr->data.ch);
                    break;
                
                case EXPRNODE_NIL:
                    add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_IMMEDIATE);
                    add_G_Bytecode_one_byte(bytecode, GINSTRDATATYPE_NIL);
                    break;

                case EXPRNODE_GLOBAL_IDENTIFIER:
                    add_G_Bytecode_one_byte(bytecode, GINSTR_LOAD_GLOBAL);
                    add_G_Bytecode_String(bytecode, &expr->data.string_identifier);
                    if (!dont_push_load) {
                        add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_LOAD);
                    }
                    break;

                case EXPRNODE_LOCAL_IDENTIFIER:
                    add_G_Bytecode_one_byte(bytecode, GINSTR_LOAD_LOCAL);
                    add_G_Bytecode_w_byte_size(bytecode, &expr->data.slot_num, sizeof(G_LOCAL_SLOT_INT));
                    if (!dont_push_load) {
                        add_G_Bytecode_one_byte(bytecode, GINSTR_PUSH_LOAD);
                    }
                    break;

                default:
                    G_log("expr G_IR_CONVERT_expression err %d:%s\n", expr->type, ExpressionNodeType_to_string(expr->type));
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

static int G_IR_ADD_G_AST_TO_BYTECODE(const G_AST astnode, G_Bytecode** addr_to_bytecode)
{
    if (!addr_to_bytecode) return IR_CONVERT_FAILED;
    if (astnode.error)     return IR_CONVERT_FAILED;

    switch (astnode.nodetype) 
    {
        case ASTNODE_DECLARATION: {
            ExpressionNodeType ir_tree = G_IR_CONVERT_expression(astnode.declarationAST.expression->top, addr_to_bytecode, false);

            if (ir_tree == EXPRNODE_UNINIT) {
                G_log("failed!\n");
                destroy_G_Bytecode_ptr(addr_to_bytecode);
                return IR_CONVERT_FAILED;
            }

            if (astnode.declarationAST.slot == 0) {
                add_G_Bytecode_one_byte(*addr_to_bytecode, (G_ubyte)GINSTR_DECLARE_GLOBAL);

                // metadata for the declaration
                add_G_Bytecode_one_byte(*addr_to_bytecode, (G_ubyte)ASTDatatype_to_G_Bytecode_Datatype(astnode.declarationAST.info.datatype));
                // str of global name
                G_uint64 length = astnode.declarationAST.info.identifier.length;
                // DO NOT FORGET TO CHANGE BELOW sizeof(datatype) FOR length PLEASE
                add_G_Bytecode_w_byte_size(*addr_to_bytecode, &length, sizeof(G_uint64));
                add_G_Bytecode_w_byte_size(*addr_to_bytecode, astnode.declarationAST.info.identifier.content, length);
            } else {
                add_G_Bytecode_one_byte(*addr_to_bytecode, (G_ubyte)GINSTR_DECLARE_LOCAL);

                // metadata for the declaration
                add_G_Bytecode_one_byte(*addr_to_bytecode, (G_ubyte)ASTDatatype_to_G_Bytecode_Datatype(astnode.declarationAST.info.datatype));

                // designate the slot we plan to target
                G_LOCAL_SLOT_INT slotnum = astnode.declarationAST.slot-1;
                // DO NOT FORGET TO CHANGE BELOW sizeof(datatype) FOR slotnum PLEASE
                add_G_Bytecode_w_byte_size(*addr_to_bytecode, &slotnum, sizeof(G_LOCAL_SLOT_INT));
            }
            break;
        }

        case ASTNODE_IF: {
            ExpressionNodeType ir_tree = G_IR_CONVERT_expression(astnode.ifWhileAST.expression->top, addr_to_bytecode, false);
            if (ir_tree == EXPRNODE_UNINIT) {
                G_log("failed ir tree!\n");
                destroy_G_Bytecode_ptr(addr_to_bytecode);
                return IR_CONVERT_FAILED;
            }

            G_Bytecode* if_conv = G_IR_CONVERT_ASTSCOPE(&astnode.ifWhileAST.nodes);
            if (!if_conv) {
                G_log("failed to get if conversion!\n");
                destroy_G_Bytecode_ptr(addr_to_bytecode);
                return IR_CONVERT_FAILED;
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
                add_G_Bytecode_one_byte(*addr_to_bytecode, (G_ubyte)GINSTR_JNT);
                G_int16 len = (G_int16)if_conv->length;
                add_G_Bytecode_w_byte_size(*addr_to_bytecode, (G_ubyte*)&len, sizeof(G_int16));
            } else {
                add_G_Bytecode_one_byte(*addr_to_bytecode, (G_ubyte)GINSTR_JNTL);
                G_int64 len = (G_int64)if_conv->length;
                add_G_Bytecode(*addr_to_bytecode, (G_ubyte*)&len, sizeof(G_int64));
            }

            add_G_Bytecode_w_byte_size(*addr_to_bytecode, if_conv->bytecode, if_conv->length);
            destroy_G_Bytecode_ptr(&if_conv);
            break;
        }

        case ASTNODE_WHILE: {
            G_Bytecode* ir_tree_bc = init_G_Bytecode_ptr();
            ExpressionNodeType ir_tree = G_IR_CONVERT_expression(astnode.ifWhileAST.expression->top, &ir_tree_bc, false);
            if (ir_tree == EXPRNODE_UNINIT) {
                G_log("failed ir tree!\n");
                destroy_G_Bytecode_ptr(&ir_tree_bc);
                destroy_G_Bytecode_ptr(addr_to_bytecode);
                return IR_CONVERT_FAILED;
            }

            G_Bytecode* while_conv = G_IR_CONVERT_ASTSCOPE(&astnode.ifWhileAST.nodes);
            if (!while_conv) {
                G_log("failed to get while conversion!\n");
                destroy_G_Bytecode_ptr(&ir_tree_bc);
                destroy_G_Bytecode_ptr(addr_to_bytecode);
                return IR_CONVERT_FAILED;
            }

            add_G_Bytecode_w_byte_size(*addr_to_bytecode, ir_tree_bc->bytecode, ir_tree_bc->length);
            if (while_conv->length <= 32766 - (2*(sizeof(G_int16) + sizeof(G_ubyte)) + ir_tree_bc->length)) {
                add_G_Bytecode_one_byte(while_conv, (G_ubyte)GINSTR_JMP);

                // for the backwards jump
                G_int16 len = -(while_conv->length + sizeof(G_ubyte) + 2*sizeof(G_int16) + ir_tree_bc->length);
                add_G_Bytecode_w_byte_size(while_conv, (G_ubyte*)&len, sizeof(G_int16));

                // we get rid of the size of the JNT + the IR tree in order to jump past the while scope properly
                len = -(len + sizeof(G_ubyte) + sizeof(G_int16) + ir_tree_bc->length);

                add_G_Bytecode_one_byte(*addr_to_bytecode, (G_ubyte)GINSTR_JNT);
                add_G_Bytecode_w_byte_size(*addr_to_bytecode, (G_ubyte*)&len, sizeof(G_int16));
            } else {
                add_G_Bytecode_one_byte(while_conv, (G_ubyte)GINSTR_JMP);

                // for the backwards jump
                G_int64 len = -(while_conv->length + sizeof(G_ubyte) + 2*sizeof(G_int64) + ir_tree_bc->length);
                add_G_Bytecode_w_byte_size(while_conv, (G_ubyte*)&len, sizeof(G_int64));

                // we get rid of the size of the JNT + the IR tree in order to jump past the while scope properly
                len = -(len + sizeof(G_ubyte) + sizeof(G_int64) + ir_tree_bc->length);

                add_G_Bytecode_one_byte(*addr_to_bytecode, (G_ubyte)GINSTR_JNT);
                add_G_Bytecode_w_byte_size(*addr_to_bytecode, (G_ubyte*)&len, sizeof(G_int64));
            }


            add_G_Bytecode_w_byte_size(*addr_to_bytecode, while_conv->bytecode, while_conv->length);
            destroy_G_Bytecode_ptr(&while_conv);
            destroy_G_Bytecode_ptr(&ir_tree_bc);
            break;
        }

        case ASTNODE_SCOPE: {
            G_Bytecode* scope_conv = G_IR_CONVERT_ASTSCOPE(&astnode.scopeAST);
            if (!scope_conv) {
                G_log("failed to get scope conversion!\n");
                destroy_G_Bytecode_ptr(&scope_conv);
                destroy_G_Bytecode_ptr(addr_to_bytecode);
                return IR_CONVERT_FAILED;
            }
            add_G_Bytecode_w_byte_size(*addr_to_bytecode, scope_conv->bytecode, scope_conv->length);
            destroy_G_Bytecode_ptr(&scope_conv);
            break;
        }


        case ASTNODE_CLEAR_LOCAL_SLOT: {
            G_log("CLEAR\n");
            add_G_Bytecode_one_byte(*addr_to_bytecode, (G_ubyte)GINSTR_CLEAR_LOCAL);
            add_G_Bytecode_w_byte_size(*addr_to_bytecode, &astnode.clearLocalSlotAST.slot, sizeof(G_LOCAL_SLOT_INT));
        }

        case ASTNODE_IGNORE:
            break;

        case ASTNODE_END:
            return IR_CONVERT_FINISHED;

        default:
            printf("IDK HOW THE IR IS MEANT TO HANDLE THIS ONE ERROR ERROR!!!!!!!WKAJDLKAWIFJ (%d)\n", astnode.nodetype);
            exit(1);
    }

    G_log("reached the end\n");
    return IR_CONVERT_CONTINUE;
}

// assumes its in a local scope
static G_Bytecode* G_IR_CONVERT_ASTSCOPE(const ASTScope* astscope)
{
    G_Bytecode* bytecode = init_G_Bytecode_ptr();
    if (!bytecode) return NULL;

    for (size_t i = 0; i < astscope->length; ++i) {
        G_log("attempt %zu:%zu:\n", i, astscope->length);
        int code = G_IR_ADD_G_AST_TO_BYTECODE(astscope->nodes[i], &bytecode);
        
        G_log("code: %d\n", code);

        // error
        if (code == IR_CONVERT_FAILED) {
            destroy_G_Bytecode_ptr(&bytecode);
            return NULL;   
        }

        // end
        if (code == IR_CONVERT_FINISHED) {
            break;
        }
    }

    return bytecode;
}

G_Bytecode* G_IR_CONVERT(G_IR* ir, G_ubyte SIZE_T_OF_PLATFORM, const bool on_global)
{
    ///////////////////////////////////////////////////////////
    ///                                                     ///
    /// SIZE_T_OF_PLATFORM GETS TEMP CHANGE PLS CHANGE BACK ///
    ///                                                     ///
    ///////////////////////////////////////////////////////////
    SIZE_T_OF_PLATFORM = 8;

    ParseState pState = init_ParseState(&ir->source);
    G_Bytecode* bytecode = init_G_Bytecode_ptr();
    G_AST astnode = (G_AST){0};

if (bytecode) {
    { // MAGIC
        if (on_global) add_G_Bytecode_w_byte_size(bytecode, "$GUINEA", 7);
    }
    bool reached_the_end = false;
    bool error = false;
    while (!reached_the_end && !error) {
        G_log_push_layer();
        G_AST astnode = parse_segment(&pState, TK_EOF, true);
        G_log_pop_layer();

        if (astnode.error) {
            if (pState.current.type == TK_Identifier)
                std_err_message(stdout, ir->filename, ir->source.content, pState.errmsg, "", pState.current.line, pState.current.column, pState.current.string.length);
            else
                std_err_message(stdout, ir->filename, ir->source.content, pState.errmsg, "", pState.current.line, pState.current.column, pState.current.length);
            error = true;
            destroy_G_Bytecode_ptr(&bytecode);
            break;
        }

        int code = G_IR_ADD_G_AST_TO_BYTECODE(astnode, &bytecode);
        
        // error
        if (code == IR_CONVERT_FAILED) {
            error = true;
        }

        // 1 == continue

        // end
        if (code == IR_CONVERT_FINISHED) {
            reached_the_end = true;
        }

        destroy_G_AST(&astnode);
    }
}
    destroy_ParseState(&pState);
    // means it wasnt erased
    if (astnode.nodetype != ASTNODE_IGNORE) destroy_G_AST(&astnode);
    return bytecode;
}

#endif