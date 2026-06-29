#ifndef AST_C
#define AST_C

#include "ast.h"
#include "../../etc/strings.h"
#include "../../etc/strings.h"
#include "../bytecode.h"
#include "../../etc/log.h"

G_byte get_pathway_count_of_ExpressionNodeAST(ExpressionNodeType type)
{
    G_byte pathways;
    switch (type)
    {
        case EXPRNODE_ADD:          pathways = 2; break;
        case EXPRNODE_SUB:          pathways = 2; break;
        case EXPRNODE_MUL:          pathways = 2; break;
        case EXPRNODE_DIV:          pathways = 2; break;
        case EXPRNODE_MOD:          pathways = 2; break;
        case EXPRNODE_POW:          pathways = 2; break;

        case EXPRNODE_AND:          pathways = 2; break;
        case EXPRNODE_OR:           pathways = 2; break;

        case EXPRNODE_EQU:          pathways = 2; break;
        case EXPRNODE_NOT_EQU:      pathways = 2; break;

        case EXPRNODE_GT:           pathways = 2; break;
        case EXPRNODE_LT:           pathways = 2; break;
        case EXPRNODE_GT_EQU:       pathways = 2; break;
        case EXPRNODE_LT_EQU:       pathways = 2; break;

        case EXPRNODE_NEG:          pathways = 1; break;
        case EXPRNODE_NOT:          pathways = 1; break;
        case EXPRNODE_CALL:         pathways = 1; break;
        case EXPRNODE_PARENTHESIS:  pathways = 1; break;
        default: pathways = 0;
    }
    return pathways;
}

char* ExpressionNodeType_to_string(ExpressionNodeType dt)
{
    switch (dt)
    {
        case EXPRNODE_UNINIT:               return "UNINIT";

        case EXPRNODE_ADD:                  return "+";
        case EXPRNODE_SUB:                  return "-";
        case EXPRNODE_MUL:                  return "*";
        case EXPRNODE_DIV:                  return "/";
        case EXPRNODE_POW:                  return "^";
        case EXPRNODE_MOD:                  return "%";

        case EXPRNODE_AND:                  return "and";
        case EXPRNODE_OR:                   return "or";
        case EXPRNODE_EQU:                  return "==";
        case EXPRNODE_NOT_EQU:              return "!=";
        case EXPRNODE_GT:                   return ">";
        case EXPRNODE_LT:                   return "<";
        case EXPRNODE_GT_EQU:               return ">=";
        case EXPRNODE_LT_EQU:               return "<=";

        case EXPRNODE_NEG:                  return "-";
        case EXPRNODE_CALL:                 return "()";
        case EXPRNODE_INT:                  return "(int)";
        case EXPRNODE_NUMBER:               return "(number)";
        case EXPRNODE_STRING:               return "(string)";
        case EXPRNODE_BOOL:                 return "(bool)";
        case EXPRNODE_CHAR:                 return "(char)";
        case EXPRNODE_FUNCTION_LITERAL:     return "(function)";
        case EXPRNODE_IDENTIFIER:           return "(identifier)";
        case EXPRNODE_NIL:                  return "nil";
        default:                            return "NULL";
    }
}




bool assign_ExpressionNodeAST(ExpressionNodeAST** x, ExpressionNodeType type)
{
    if (!x) return false;
    if (!(*x)) {
        *x = malloc(sizeof(ExpressionNodeAST));
        if (!(*x)) return false;
        **x = (ExpressionNodeAST){0};
    }
    (*x)->type = type;
    return true;
}


void G_log_ExpressionNodeAST(ExpressionNodeAST* x)
{
    if (!x) {
        G_log("NULL\n");
        return;
    }

    switch (x->type)
    {
        case EXPRNODE_UNINIT: return;

        case EXPRNODE_ADD:
            G_log("+\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        case EXPRNODE_SUB:
            G_log("-\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        case EXPRNODE_MUL:
            G_log("*\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        case EXPRNODE_DIV:
            G_log("/\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        case EXPRNODE_POW:
            G_log("^\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        case EXPRNODE_MOD:
            G_log("%\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;


        ///

        case EXPRNODE_AND:
            G_log("and\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        case EXPRNODE_OR:
            G_log("or\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        case EXPRNODE_EQU:
            G_log("==\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        case EXPRNODE_NOT_EQU:
            G_log("!=\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        case EXPRNODE_GT:
            G_log(">\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        case EXPRNODE_LT:
            G_log("<\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        case EXPRNODE_GT_EQU:
            G_log(">=\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        case EXPRNODE_LT_EQU:
            G_log("<=\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_ExpressionNodeAST(x->left);
            G_log_pop_layer();
            return;

        ///
        

        case EXPRNODE_NEG:
            G_log("NEG\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_pop_layer();
            return;

        case EXPRNODE_PARENTHESIS:
            G_log("(...)\n");
            G_log_push_layer();
            G_log_ExpressionNodeAST(x->right);
            G_log_pop_layer();
            return;

        case EXPRNODE_CALL:
            G_log("(");

            for (size_t i = 0; i < x->data.exprFuncCallAST.expression_args_length; i++){
                G_log_ExpressionNodeAST(x->data.exprFuncCallAST.expression_args[i].top);

                if (i != x->data.exprFuncCallAST.expression_args_length-1)
                    G_log(", ");
            }

            G_log(")\n");
            return;

        case EXPRNODE_INT:
            G_log("int:%lld\n", x->data.integer);
            return;

        case EXPRNODE_CHAR:
            G_log("'%c'\n", x->data.ch);
            return;

        case EXPRNODE_BOOL:     
            G_log("%s\n", (x->data.bl) ? " true" : " false");
            return;

        case EXPRNODE_NUMBER:
            G_log("%lf\n", x->data.number);
            return;

        case EXPRNODE_STRING:
            G_log("\"%s\"\n", x->data.string_identifier.content);
            return;

        case EXPRNODE_IDENTIFIER:
            G_log("%s\n", x->data.string_identifier.content);
            return;

        case EXPRNODE_FUNCTION_LITERAL:
            G_log("(function)\n");
            return;

        case EXPRNODE_NIL:
            G_log("nil\n");
            return;

        default:
            G_log("![Error (%d:%s)]\n", x->type, ExpressionNodeType_to_string(x->type));
    }
}

// FOR DESTROYING ExpressionNodeAST*
void destroy_ExpressionNodeAST_ptr(ExpressionNodeAST** x)
{
    if (!x) return;
    ExpressionNodeAST* tmp = *x;

    if (tmp) {
        switch (tmp->type)
        {
            case EXPRNODE_UNINIT: break;

            case EXPRNODE_ADD:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_SUB:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_MUL:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_DIV:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_POW:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_MOD:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;


            ////////////////////////////////////////////////////////
            // LOGIC
            case EXPRNODE_AND:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_OR:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_EQU:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_NOT_EQU:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_GT:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_LT:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_GT_EQU:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_LT_EQU:
                destroy_ExpressionNodeAST_ptr(&tmp->left);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;
            ////////////////////////////////////////////////////////



            case EXPRNODE_NEG:
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_PARENTHESIS:
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_CALL:
                destroy_ExprFuncCallAST(&tmp->data.exprFuncCallAST);
                destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case EXPRNODE_INT:      break;
            case EXPRNODE_CHAR:     break;
            case EXPRNODE_BOOL:     break;
            case EXPRNODE_NUMBER:   break;

            case EXPRNODE_STRING:
                clearstring(&tmp->data.string_identifier);
                break;

            case EXPRNODE_IDENTIFIER:
                clearstring(&tmp->data.string_identifier);
                break;

            case EXPRNODE_FUNCTION_LITERAL:
                destroy_FunctionAST(&tmp->data.function);
                break;

            case EXPRNODE_NIL:
                break;

            default:
                printf("Error: ExpressionNodeAST failed to destroy! (%d:%s)\n", tmp->type, ExpressionNodeType_to_string(tmp->type));
                exit(1);
        }
    }
    if (tmp) free(tmp);
    *x = NULL;
}

ExpressionAST* init_ExpressionAST_ptr()
{
    ExpressionAST* x = malloc(sizeof(ExpressionAST));
    if (x) *x = (ExpressionAST){0};
    return x;
}

// FOR DESTROYING ExpressionAST
void destroy_ExpressionAST(ExpressionAST* x)
{
    if (!x) return;
    destroy_ExpressionNodeAST_ptr(&x->top);
    x->fail = false;
}

// FOR DESTROYING ExpressionAST*
void destroy_ExpressionAST_ptr(ExpressionAST** x)
{
    if (!x) return;
    destroy_ExpressionAST(*x);
    if (*x) free(*x);
    *x = NULL;
}




void destroy_VariableDeclaration(VariableDeclarationAST* x)
{
    if (!x) return;
    destroy_ExpressionAST_ptr(&x->expression);
    clearstring(&x->info.identifier);
}

void destroy_VariableInfoAST(VariableInfoAST* x)
{
    if (!x) return;
    clearstring(&x->identifier);
    x->datatype = ASTDATATYPE_ERR;
}

void destroy_VariableAssignAST(VariableAssignAST* x)
{
    if (!x) return;
    destroy_ExpressionAST_ptr(&x->expression);
    clearstring(&x->to_assign);
}

void destroy_ExprFuncCallAST(ExprFuncCallAST* x)
{
    if (x->expression_args != NULL) {
        for (size_t i = 0; i < x->expression_args_length; i++) {
            destroy_ExpressionAST(x->expression_args + i * sizeof(ExpressionAST));
        }
        free(x->expression_args);
        x->expression_args = NULL;
    }
    x->expression_args_length = 0;
}


void destroy_FunctionAST(FunctionAST* x)
{
    if (!x) return;
    destroy_G_Bytecode(&x->bytecode);
    if (x->args.args) free(x->args.args);
    x->args.length = 0;
    x->args.size   = 0;
}




ASTScope init_ASTScope()
{
    ASTScope x = (ASTScope){0};
    x.size = 6;
    x.nodes = malloc(sizeof(G_AST) * x.size);
    if (!x.nodes) {
        return (ASTScope){0};
    }
    return x;
}

// does NOT deepcopy pointers in it, just a lightcopy, BEWARE
bool add_G_AST_to_ASTScope(ASTScope* x, G_AST toadd)
{
    while (x->length >= x->size) {
        x->size *= 2;
        G_AST* tmp = realloc(x->nodes, x->size * sizeof(ASTScope));
        if (!tmp) {
            x->size /= 2;
            return false;
        }
        x->nodes = tmp;
    }
    x->nodes[x->length++] = toadd;
    return true;
}

void destroy_ASTScope(ASTScope* x)
{
    if (!x) return;
    if (!x->nodes) return;

    for (size_t i = 0; i < x->length; ++i) {
        destroy_G_AST(&x->nodes[i]);
    }

    free(x->nodes);
    x->nodes  = NULL;
    x->length = 0;
    x->size   = 0;
}



IfWhileAST init_IfWhileAST()
{
    IfWhileAST x = (IfWhileAST){0};
    x.nodes = init_ASTScope();
    return x;
}

void destroy_IfWhileAST(IfWhileAST* x)
{
    G_log("doing\n");
    destroy_ExpressionAST_ptr(&x->expression);
    G_log("finish\n");
    destroy_ASTScope(&x->nodes);
    G_log("finish\n");
}



void destroy_G_AST(G_AST* g_ast)
{
    G_log("destroying G_AST...\n");
    switch (g_ast->nodetype)
    {
        case ASTNODE_DECLARATION: { destroy_VariableDeclaration(&g_ast->declarationAST); break; }
        case ASTNODE_ASSIGN:      { destroy_VariableAssignAST(&g_ast->assignAST); break;        }
        case ASTNODE_IF:          { destroy_IfWhileAST(&g_ast->ifWhileAST); break;              }
        case ASTNODE_WHILE:       { destroy_IfWhileAST(&g_ast->ifWhileAST); break;              }

        case ASTNODE_END:     G_log("end\n");    break;
        case ASTNODE_IGNORE:  G_log("ignore\n"); break;
        
        default: printf("err ASTNODE G_AST destroy\n"); exit(1);
    }
    g_ast->nodetype = ASTNODE_IGNORE;
    G_log("done destroying G_AST\n");
}

#endif