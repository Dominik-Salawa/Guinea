#ifndef AST_C
#define AST_C

#include "ast.h"
#include "../../etc/strings.h"
#include "../../etc/strings.h"
#include "../bytecode.h"
#include "../../etc/log.h"


char* GUIN_ASTDatatype_to_string(GUIN_ASTDatatype dt)
{
    switch (dt)
    {
        case GUIN_ASTDATATYPE_ERR:          return "err";
        case GUIN_ASTDATATYPE_NIL:          return "nil";
        case GUIN_ASTDATATYPE_INT:          return "int";
        case GUIN_ASTDATATYPE_NUMBER:       return "number";
        case GUIN_ASTDATATYPE_BOOL:         return "bool";
        case GUIN_ASTDATATYPE_FUNCTION:     return "function";
        case GUIN_ASTDATATYPE_STRING:       return "string";
        case GUIN_ASTDATATYPE_CHAR:         return "char";
        case GUIN_ASTDATATYPE_DYNAMIC:      return "dynamic";
        default:                            return "(null)";
    }
}



GUIN_byte GUIN_get_pathway_count_of_ExpressionNodeAST(GUIN_ExpressionNodeType type)
{
    GUIN_byte pathways;
    switch (type)
    {
        case GUIN_EXPRNODE_ADD:          pathways = 2; break;
        case GUIN_EXPRNODE_SUB:          pathways = 2; break;
        case GUIN_EXPRNODE_MUL:          pathways = 2; break;
        case GUIN_EXPRNODE_DIV:          pathways = 2; break;
        case GUIN_EXPRNODE_MOD:          pathways = 2; break;
        case GUIN_EXPRNODE_POW:          pathways = 2; break;

        case GUIN_EXPRNODE_AND:          pathways = 2; break;
        case GUIN_EXPRNODE_OR:           pathways = 2; break;

        case GUIN_EXPRNODE_EQU:          pathways = 2; break;
        case GUIN_EXPRNODE_NOT_EQU:      pathways = 2; break;

        case GUIN_EXPRNODE_GT:           pathways = 2; break;
        case GUIN_EXPRNODE_LT:           pathways = 2; break;
        case GUIN_EXPRNODE_GT_EQU:       pathways = 2; break;
        case GUIN_EXPRNODE_LT_EQU:       pathways = 2; break;

        case GUIN_EXPRNODE_NEG:          pathways = 1; break;
        case GUIN_EXPRNODE_NOT:          pathways = 1; break;
        case GUIN_EXPRNODE_CALL:         pathways = 1; break;
        case GUIN_EXPRNODE_PARENTHESIS:  pathways = 1; break;
        default: pathways = 0;
    }
    return pathways;
}

char* GUIN_ExpressionNodeType_to_string(GUIN_ExpressionNodeType dt)
{
    switch (dt)
    {
        case GUIN_EXPRNODE_UNINIT:               return "UNINIT";

        case GUIN_EXPRNODE_ADD:                  return "+";
        case GUIN_EXPRNODE_SUB:                  return "-";
        case GUIN_EXPRNODE_MUL:                  return "*";
        case GUIN_EXPRNODE_DIV:                  return "/";
        case GUIN_EXPRNODE_POW:                  return "^";
        case GUIN_EXPRNODE_MOD:                  return "%";

        case GUIN_EXPRNODE_AND:                  return "and";
        case GUIN_EXPRNODE_OR:                   return "or";
        case GUIN_EXPRNODE_EQU:                  return "==";
        case GUIN_EXPRNODE_NOT_EQU:              return "!=";
        case GUIN_EXPRNODE_GT:                   return ">";
        case GUIN_EXPRNODE_LT:                   return "<";
        case GUIN_EXPRNODE_GT_EQU:               return ">=";
        case GUIN_EXPRNODE_LT_EQU:               return "<=";

        case GUIN_EXPRNODE_NEG:                  return "-";
        case GUIN_EXPRNODE_CALL:                 return "()";
        case GUIN_EXPRNODE_INT:                  return "(int)";
        case GUIN_EXPRNODE_NUMBER:               return "(number)";
        case GUIN_EXPRNODE_STRING:               return "(string)";
        case GUIN_EXPRNODE_BOOL:                 return "(bool)";
        case GUIN_EXPRNODE_CHAR:                 return "(char)";
        case GUIN_EXPRNODE_FUNCTION_LITERAL:     return "(function)";
        case GUIN_EXPRNODE_GLOBAL_IDENTIFIER:    return "(global identifier)";
        case GUIN_EXPRNODE_LOCAL_IDENTIFIER:     return "(local identifier)";
        case GUIN_EXPRNODE_NIL:                  return "nil";
        default:                            return "NULL";
    }
}




bool GUIN_assign_ExpressionNodeAST(GUIN_ExpressionNodeAST** x, GUIN_ExpressionNodeType type)
{
    if (!x) return false;
    if (!(*x)) {
        *x = malloc(sizeof(GUIN_ExpressionNodeAST));
        if (!(*x)) return false;
        **x = (GUIN_ExpressionNodeAST){0};
    }
    (*x)->type = type;
    return true;
}


void GUIN_log_ExpressionNodeAST(GUIN_ExpressionNodeAST* x)
{
    if (!x) {
        GUIN_log("NULL\n");
        return;
    }

    switch (x->type)
    {
        case GUIN_EXPRNODE_UNINIT: return;

        case GUIN_EXPRNODE_ADD:
            GUIN_log("+\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_SUB:
            GUIN_log("-\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_MUL:
            GUIN_log("*\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_DIV:
            GUIN_log("/\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_POW:
            GUIN_log("^\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_MOD:
            GUIN_log("%\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;


        ///

        case GUIN_EXPRNODE_AND:
            GUIN_log("and\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_OR:
            GUIN_log("or\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_NOT:
            GUIN_log("not\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_EQU:
            GUIN_log("==\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_NOT_EQU:
            GUIN_log("!=\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_GT:
            GUIN_log(">\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_LT:
            GUIN_log("<\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_GT_EQU:
            GUIN_log(">=\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_LT_EQU:
            GUIN_log("<=\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_ExpressionNodeAST(x->left);
            GUIN_log_pop_layer();
            return;

        ///
        

        case GUIN_EXPRNODE_NEG:
            GUIN_log("NEG\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_PARENTHESIS:
            GUIN_log("(...)\n");
            GUIN_log_push_layer();
            GUIN_log_ExpressionNodeAST(x->right);
            GUIN_log_pop_layer();
            return;

        case GUIN_EXPRNODE_CALL:
            GUIN_log("(");

            for (size_t i = 0; i < x->data.exprFuncCallAST.expression_args_length; i++){
                GUIN_log_ExpressionNodeAST(x->data.exprFuncCallAST.expression_args[i]->top);

                if (i != x->data.exprFuncCallAST.expression_args_length-1)
                    GUIN_log(", ");
            }

            GUIN_log(")\n");
            return;

        case GUIN_EXPRNODE_INT:
            GUIN_log("int:%lld\n", x->data.integer);
            return;

        case GUIN_EXPRNODE_CHAR:
            GUIN_log("'%c'\n", x->data.ch);
            return;

        case GUIN_EXPRNODE_BOOL:     
            GUIN_log("%s\n", (x->data.bl) ? " true" : " false");
            return;

        case GUIN_EXPRNODE_NUMBER:
            GUIN_log("%lf\n", x->data.number);
            return;

        case GUIN_EXPRNODE_STRING:
            GUIN_log("\"%s\"\n", x->data.string_identifier.content);
            return;

        case GUIN_EXPRNODE_GLOBAL_IDENTIFIER:
            GUIN_log("(global) %s\n", x->data.string_identifier.content);
            return;

        case GUIN_EXPRNODE_LOCAL_IDENTIFIER:
            GUIN_log("(local) %d\n", x->data.slot_num);
            return;

        case GUIN_EXPRNODE_FUNCTION_LITERAL:
            GUIN_log("(function)\n");
            return;

        case GUIN_EXPRNODE_NIL:
            GUIN_log("nil\n");
            return;
    }
}

// FOR DESTROYING ExpressionNodeAST*
void GUIN_destroy_ExpressionNodeAST_ptr(GUIN_ExpressionNodeAST** x)
{
    if (!x) return;
    GUIN_ExpressionNodeAST* tmp = *x;

    if (tmp) {
        switch (tmp->type)
        {
            case GUIN_EXPRNODE_UNINIT: break;

            case GUIN_EXPRNODE_ADD:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_SUB:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_MUL:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_DIV:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_POW:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_MOD:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;


            ////////////////////////////////////////////////////////
            // LOGIC
            case GUIN_EXPRNODE_AND:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_OR:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_NOT:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_EQU:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_NOT_EQU:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_GT:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_LT:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_GT_EQU:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_LT_EQU:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->left);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;
            ////////////////////////////////////////////////////////



            case GUIN_EXPRNODE_NEG:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_PARENTHESIS:
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case GUIN_EXPRNODE_CALL:
            printf("call\n");
                GUIN_destroy_ExprFuncCallAST(&tmp->data.exprFuncCallAST);
            printf("continue\n");
                GUIN_log_ExpressionNodeAST(tmp->right);
                GUIN_destroy_ExpressionNodeAST_ptr(&tmp->right);
            printf("done\n");
                break;

            case GUIN_EXPRNODE_NIL:      break;
            case GUIN_EXPRNODE_INT:      break;
            case GUIN_EXPRNODE_CHAR:     break;
            case GUIN_EXPRNODE_BOOL:     break;
            case GUIN_EXPRNODE_NUMBER:   break;

            case GUIN_EXPRNODE_STRING:
                GUIN_clearstring(&tmp->data.string_identifier);
                break;

            case GUIN_EXPRNODE_GLOBAL_IDENTIFIER:
            printf("identifier\n");
                GUIN_clearstring(&tmp->data.string_identifier);
                break;

            case GUIN_EXPRNODE_LOCAL_IDENTIFIER:
                break;

            case GUIN_EXPRNODE_FUNCTION_LITERAL:
                GUIN_destroy_FunctionAST(&tmp->data.function);
                break;
        }
    }
    if (tmp) free(tmp);
    *x = NULL;
}

GUIN_ExpressionAST* GUIN_init_ExpressionAST_ptr(void)
{
    GUIN_ExpressionAST* x = malloc(sizeof(GUIN_ExpressionAST));
    if (x) *x = (GUIN_ExpressionAST){0};
    return x;
}

// FOR DESTROYING ExpressionAST
void GUIN_destroy_ExpressionAST(GUIN_ExpressionAST* x)
{
    if (!x) return;
    GUIN_destroy_ExpressionNodeAST_ptr(&x->top);
    x->fail = false;
}

// FOR DESTROYING ExpressionAST*
void GUIN_destroy_ExpressionAST_ptr(GUIN_ExpressionAST** x)
{
    if (!x) return;
    GUIN_destroy_ExpressionAST(*x);
    if (*x) free(*x);
    *x = NULL;
}




void GUIN_destroy_VariableDeclarationAST(GUIN_VariableDeclarationAST* x)
{
    if (!x) return;
    GUIN_destroy_ExpressionAST_ptr(&x->expression);
    GUIN_clearstring(&x->info.identifier);
}

void GUIN_destroy_VariableInfoAST(GUIN_VariableInfoAST* x)
{
    if (!x) return;
    GUIN_clearstring(&x->identifier);
    x->datatype = GUIN_ASTDATATYPE_ERR;
}


GUIN_ExprFuncCallAST GUIN_init_ExprFuncCallAST(void)
{
    GUIN_ExprFuncCallAST x = {0};
    x.expression_args_size = 8;
    x.expression_args = malloc(sizeof(GUIN_ExpressionAST*) * x.expression_args_size);
    if (!x.expression_args) return (GUIN_ExprFuncCallAST){0};
    return x;
}
// does not deep copy `toadd`
bool GUIN_add_ExpressionAST_ptr_to_ExprFuncCallAST(GUIN_ExprFuncCallAST* x, GUIN_ExpressionAST* toadd)
{
    if (x->expression_args_length >= x->expression_args_size) {
        size_t original_size = x->expression_args_size;
        while (x->expression_args_length >= x->expression_args_size) 
            x->expression_args_size *= 2;
        GUIN_ExpressionAST** tmp = realloc(x->expression_args, sizeof(GUIN_ExpressionAST*) * x->expression_args_size);
        if (!tmp) {
            x->expression_args_size = original_size;
            return false;
        }
        x->expression_args = tmp;
    }

    x->expression_args[x->expression_args_length++] = toadd;
    return true;
}
void GUIN_destroy_ExprFuncCallAST(GUIN_ExprFuncCallAST* x)
{
    if (!x) return;
    if (x->expression_args) {
        for (size_t i = 0; i < x->expression_args_length; ++i)
            GUIN_destroy_ExpressionAST(x->expression_args[i]);

        free(x->expression_args);
        x->expression_args = NULL;
    }
    x->expression_args_length = 0;
}

GUIN_FunctionAST GUIN_init_FunctionAST(void)
{
    GUIN_FunctionAST x = (GUIN_FunctionAST){0};
    x.args.size = 3;
    x.args.args = malloc(sizeof(GUIN_FuncArgsAST) * x.args.size);
    if (!x.args.args) return (GUIN_FunctionAST){0};
    return x;
}
void GUIN_destroy_FunctionAST(GUIN_FunctionAST* x)
{
    if (!x) return;
    GUIN_destroy_ASTScope(&x->scope);
    if (x->args.args) free(x->args.args);
    GUIN_clearstring_ptr(&x->name);
    x->args.length = 0;
    x->args.size   = 0;
}
bool GUIN_add_arg_to_FunctionAST(GUIN_FunctionAST* x, GUIN_ASTDatatype datatype)
{
    if (!x) return false;

    if (x->args.length >= x->args.size) {
        size_t original_size = x->args.size;
        while (x->args.length >= x->args.size) x->args.size *= 2;
        GUIN_FuncArgsAST* tmp = realloc(x->args.args, sizeof(GUIN_FuncArgsAST) * x->args.size);
        if (!tmp) {
            x->args.size = original_size;
            return false;
        }
        x->args.args = tmp;
    }

    x->args.args[x->args.length++].type = datatype;
    return true;
}



GUIN_ASTScope GUIN_init_ASTScope(void)
{
    GUIN_ASTScope x = (GUIN_ASTScope){0};
    x.size = 6;
    x.nodes = malloc(sizeof(GUIN_AST) * x.size);
    if (!x.nodes) {
        return (GUIN_ASTScope){0};
    }
    return x;
}

// does NOT deepcopy pointers in it, just a lightcopy, BEWARE
bool GUIN_add_AST_to_ASTScope(GUIN_ASTScope* x, GUIN_AST* toadd)
{
    if (!x) return false;

    if (x->length >= x->size) {
        size_t original_size = x->size;
        if (x->size > 0) {
            while (x->length >= x->size) x->size *= 2;
        } else {
            x->size = 32;
        }
        GUIN_AST* tmp = realloc(x->nodes, x->size * sizeof(GUIN_AST));
        if (!tmp) {
            x->size = original_size;
            return false;
        }
        x->nodes = tmp;
    }
    x->nodes[x->length++] = *toadd;
    return true;
}

void GUIN_destroy_ASTScope(GUIN_ASTScope* x)
{
    if (!x) return;
    if (!x->nodes) return;

    for (size_t i = 0; i < x->length; ++i) {
        GUIN_destroy_AST(&x->nodes[i]);
    }

    free(x->nodes);
    x->nodes  = NULL;
    x->length = 0;
    x->size   = 0;
}



GUIN_IfWhileAST GUIN_init_IfWhileAST(void)
{
    GUIN_IfWhileAST x = (GUIN_IfWhileAST){0};
    x.nodes = GUIN_init_ASTScope();
    return x;
}

void GUIN_destroy_IfWhileAST(GUIN_IfWhileAST* x)
{
    if (!x) return;
    GUIN_destroy_ExpressionAST_ptr(&x->expression);
    GUIN_destroy_ASTScope(&x->nodes);
}



GUIN_AST* GUIN_init_AST_ptr(GUIN_ASTNodeType nodetype)
{
    GUIN_AST* x = malloc(sizeof(GUIN_AST));
    *x = (GUIN_AST){0};
    if (!x) return NULL;
    x->nodetype = nodetype;
    return x;
}
void GUIN_destroy_AST(GUIN_AST* g_ast)
{
    GUIN_log("destroying GUIN_AST...\n");
    switch (g_ast->nodetype)
    {
        case GUIN_ASTNODE_DECLARATION: { GUIN_destroy_VariableDeclarationAST(&g_ast->declarationAST);      break; }
        case GUIN_ASTNODE_IF:          { GUIN_destroy_IfWhileAST(&g_ast->ifWhileAST);                      break; }
        case GUIN_ASTNODE_WHILE:       { GUIN_destroy_IfWhileAST(&g_ast->ifWhileAST);                      break; }
        case GUIN_ASTNODE_SCOPE:       { GUIN_destroy_ASTScope(&g_ast->scopeAST);                          break; }
        case GUIN_ASTNODE_RETURN:      { GUIN_destroy_ExpressionNodeAST_ptr(&g_ast->returnAST.expression); break; }

        case GUIN_ASTNODE_NULL:                  break;
        case GUIN_ASTNODE_CLEAR_LOCAL_SLOT:      break;
        case GUIN_ASTNODE_END:                   break;
        case GUIN_ASTNODE_IGNORE:                break;
        case GUIN_ASTNODE_BREAK:                 break;
        case GUIN_ASTNODE_CONTINUE:              break;
    }
    g_ast->nodetype = GUIN_ASTNODE_IGNORE;
    GUIN_log("done destroying GUIN_AST\n");
}
void GUIN_destroy_AST_ptr(GUIN_AST** g_ast)
{
    if (!g_ast) return;
    if (!*g_ast) return;
    GUIN_destroy_AST(*g_ast);
    free(*g_ast);
    *g_ast = NULL;
}

#endif
