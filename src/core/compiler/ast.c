#ifndef AST_C
#define AST_C

#include "ast.h"
#include "../../etc/strings.h"
#include "../../etc/strings.h"
#include "../bytecode.h"
#include "../../etc/log.h"


char* guin_ASTDatatype_to_string(guin_ASTDatatype dt)
{
    switch (dt)
    {
        case guin_ASTDATATYPE_ERR:          return "err";
        case guin_ASTDATATYPE_NIL:          return "nil";
        case guin_ASTDATATYPE_INT:          return "int";
        case guin_ASTDATATYPE_NUMBER:       return "number";
        case guin_ASTDATATYPE_BOOL:         return "bool";
        case guin_ASTDATATYPE_FUNCTION:     return "function";
        case guin_ASTDATATYPE_STRING:       return "string";
        case guin_ASTDATATYPE_CHAR:         return "char";
        case guin_ASTDATATYPE_DYNAMIC:      return "dynamic";
		case guin_ASTDATATYPE_VOID:			return "void";
    }
	return "(null)";
}



guin_byte guin_get_pathway_count_of_ExpressionNodeAST(guin_ExpressionNodeType type)
{
    guin_byte pathways;
    switch (type)
    {
        case guin_EXPRNODE_ADD:          	  pathways = 2; break;
        case guin_EXPRNODE_SUB:          	  pathways = 2; break;
        case guin_EXPRNODE_MUL:          	  pathways = 2; break;
        case guin_EXPRNODE_DIV:          	  pathways = 2; break;
        case guin_EXPRNODE_MOD:          	  pathways = 2; break;
        case guin_EXPRNODE_POW:          	  pathways = 2; break;
        case guin_EXPRNODE_ASSIGN:       	  pathways = 2; break;

        case guin_EXPRNODE_AND:          	  pathways = 2; break;
        case guin_EXPRNODE_OR:           	  pathways = 2; break;

        case guin_EXPRNODE_EQU:          	  pathways = 2; break;
        case guin_EXPRNODE_NOT_EQU:     	  pathways = 2; break;

        case guin_EXPRNODE_GT:           	  pathways = 2; break;
        case guin_EXPRNODE_LT:           	  pathways = 2; break;
        case guin_EXPRNODE_GT_EQU:       	  pathways = 2; break;
        case guin_EXPRNODE_LT_EQU:       	  pathways = 2; break;

        case guin_EXPRNODE_NEG:          	  pathways = 1; break;
        case guin_EXPRNODE_NOT:          	  pathways = 1; break;
        case guin_EXPRNODE_CALL:         	  pathways = 1; break;
        case guin_EXPRNODE_PARENTHESIS:  	  pathways = 1; break;

		case guin_EXPRNODE_INT: 		 	  pathways = 0; break;
		case guin_EXPRNODE_NUMBER: 		 	  pathways = 0; break;
		case guin_EXPRNODE_STRING: 		 	  pathways = 0; break;
		case guin_EXPRNODE_BOOL: 		 	  pathways = 0; break;
		case guin_EXPRNODE_CHAR: 		 	  pathways = 0; break;
		case guin_EXPRNODE_FUNCTION_LITERAL:  pathways = 0; break;
		case guin_EXPRNODE_GLOBAL_IDENTIFIER: pathways = 0; break;
		case guin_EXPRNODE_LOCAL_IDENTIFIER:  pathways = 0; break;
		case guin_EXPRNODE_NIL:				  pathways = 0; break;

		case guin_EXPRNODE_UNINIT:			  pathways = 0; break;
    }
    return pathways;
}

char* guin_ExpressionNodeType_to_string(guin_ExpressionNodeType dt)
{
    switch (dt)
    {
        case guin_EXPRNODE_UNINIT:               return "UNINIT";

        case guin_EXPRNODE_ADD:                  return "+";
        case guin_EXPRNODE_SUB:                  return "-";
        case guin_EXPRNODE_MUL:                  return "*";
        case guin_EXPRNODE_DIV:                  return "/";
        case guin_EXPRNODE_POW:                  return "^";
        case guin_EXPRNODE_MOD:                  return "%";

        case guin_EXPRNODE_AND:                  return "and";
        case guin_EXPRNODE_OR:                   return "or";
        case guin_EXPRNODE_EQU:                  return "==";
        case guin_EXPRNODE_NOT_EQU:              return "!=";
        case guin_EXPRNODE_GT:                   return ">";
        case guin_EXPRNODE_LT:                   return "<";
        case guin_EXPRNODE_GT_EQU:               return ">=";
        case guin_EXPRNODE_LT_EQU:               return "<=";
        case guin_EXPRNODE_ASSIGN:               return "=";
        case guin_EXPRNODE_NOT:	                 return "!";

        case guin_EXPRNODE_NEG:                  return "-";
        case guin_EXPRNODE_CALL:                 return "()";
        case guin_EXPRNODE_INT:                  return "(int)";
        case guin_EXPRNODE_NUMBER:               return "(number)";
        case guin_EXPRNODE_STRING:               return "(string)";
        case guin_EXPRNODE_BOOL:                 return "(bool)";
        case guin_EXPRNODE_CHAR:                 return "(char)";
        case guin_EXPRNODE_FUNCTION_LITERAL:     return "(function)";
        case guin_EXPRNODE_GLOBAL_IDENTIFIER:    return "(global identifier)";
        case guin_EXPRNODE_LOCAL_IDENTIFIER:     return "(local identifier)";
        case guin_EXPRNODE_NIL:                  return "nil";
		case guin_EXPRNODE_PARENTHESIS:		     return "(...)";
    }
	return "(null)";
}




bool guin_assign_ExpressionNodeAST(guin_ExpressionNodeAST** x, guin_ExpressionNodeType type)
{
    if (!x) return false;
    if (!(*x)) {
        *x = guin_malloc(sizeof(guin_ExpressionNodeAST));
        if (!(*x)) return false;
        **x = (guin_ExpressionNodeAST){0};
    }
    (*x)->type = type;
    return true;
}


void guin_log_ExpressionNodeAST(guin_ExpressionNodeAST* x)
{
    if (!x) {
        guin_log("NULL\n");
        return;
    }

    switch (x->type)
    {
        case guin_EXPRNODE_UNINIT: return;

        case guin_EXPRNODE_ADD:
            guin_log("+\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_SUB:
            guin_log("-\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_MUL:
            guin_log("*\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_DIV:
            guin_log("/\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_POW:
            guin_log("^\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_MOD:
            guin_log("%\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;


        ///

        case guin_EXPRNODE_AND:
            guin_log("and\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_OR:
            guin_log("or\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_NOT:
            guin_log("not\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_pop_layer();
            return;

		case guin_EXPRNODE_ASSIGN:
            guin_log("=\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_EQU:
            guin_log("==\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_NOT_EQU:
            guin_log("!=\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_GT:
            guin_log(">\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_LT:
            guin_log("<\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_GT_EQU:
            guin_log(">=\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_LT_EQU:
            guin_log("<=\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_ExpressionNodeAST(x->left);
            guin_log_pop_layer();
            return;

        ///

        case guin_EXPRNODE_NEG:
            guin_log("NEG\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_PARENTHESIS:
            guin_log("(...)\n");
            guin_log_push_layer();
            guin_log_ExpressionNodeAST(x->right);
            guin_log_pop_layer();
            return;

        case guin_EXPRNODE_CALL:
            guin_log("(");

            for (size_t i = 0; i < x->data.exprFuncCallAST.expression_args_length; i++){
                guin_log_ExpressionNodeAST(x->data.exprFuncCallAST.expression_args[i]->top);

                if (i != x->data.exprFuncCallAST.expression_args_length-1)
                    guin_log(", ");
            }

            guin_log(")\n");
            return;

        case guin_EXPRNODE_INT:
            guin_log("int:%lld\n", x->data.integer);
            return;

        case guin_EXPRNODE_CHAR:
            guin_log("'%c'\n", x->data.ch);
            return;

        case guin_EXPRNODE_BOOL:     
            guin_log("%s\n", (x->data.bl) ? " true" : " false");
            return;

        case guin_EXPRNODE_NUMBER:
            guin_log("%lf\n", x->data.number);
            return;

        case guin_EXPRNODE_STRING:
            guin_log("\"%s\"\n", x->data.string_identifier.content);
            return;

        case guin_EXPRNODE_GLOBAL_IDENTIFIER:
            guin_log("(global) %s\n", x->data.string_identifier.content);
            return;

        case guin_EXPRNODE_LOCAL_IDENTIFIER:
            guin_log("(local) %d\n", x->data.slot_num);
            return;

        case guin_EXPRNODE_FUNCTION_LITERAL:
            guin_log("(function)\n");
            return;

        case guin_EXPRNODE_NIL:
            guin_log("nil\n");
            return;
    }
}

// FOR DESTROYING ExpressionNodeAST*
void guin_destroy_ExpressionNodeAST_ptr(guin_ExpressionNodeAST** x)
{
    if (!x) return;
    guin_ExpressionNodeAST* tmp = *x;

    if (tmp) {
        switch (tmp->type)
        {
            case guin_EXPRNODE_UNINIT: break;

            case guin_EXPRNODE_ADD:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_SUB:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_MUL:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_DIV:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_POW:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_MOD:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;


            ////////////////////////////////////////////////////////
            // LOGIC
            case guin_EXPRNODE_AND:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_OR:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_NOT:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

			case guin_EXPRNODE_ASSIGN:
				guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_EQU:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_NOT_EQU:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_GT:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_LT:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_GT_EQU:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_LT_EQU:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->left);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;
            ////////////////////////////////////////////////////////



            case guin_EXPRNODE_NEG:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_PARENTHESIS:
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_CALL:
                guin_destroy_ExprFuncCallAST(&tmp->data.exprFuncCallAST);
                guin_log_ExpressionNodeAST(tmp->right);
                guin_destroy_ExpressionNodeAST_ptr(&tmp->right);
                break;

            case guin_EXPRNODE_NIL:      break;
            case guin_EXPRNODE_INT:      break;
            case guin_EXPRNODE_CHAR:     break;
            case guin_EXPRNODE_BOOL:     break;
            case guin_EXPRNODE_NUMBER:   break;

            case guin_EXPRNODE_STRING:
                guin_clearstring(&tmp->data.string_identifier);
                break;

            case guin_EXPRNODE_GLOBAL_IDENTIFIER:
                guin_clearstring(&tmp->data.string_identifier);
                break;

            case guin_EXPRNODE_LOCAL_IDENTIFIER:
                break;

            case guin_EXPRNODE_FUNCTION_LITERAL:
                guin_destroy_FunctionAST(&tmp->data.function);
                break;
        }
    }
    if (tmp) guin_mfree(tmp);
    *x = NULL;
}

guin_ExpressionAST* guin_init_ExpressionAST_ptr(void)
{
    guin_ExpressionAST* x = guin_malloc(sizeof(guin_ExpressionAST));
    if (x) *x = (guin_ExpressionAST){0};
    return x;
}

void guin_destroy_ExpressionAST(guin_ExpressionAST* x)
{
    if (!x) return;
    guin_destroy_ExpressionNodeAST_ptr(&x->top);
    x->fail = false;
}

void guin_destroy_ExpressionAST_ptr(guin_ExpressionAST** x)
{
    if (!x) return;
    guin_destroy_ExpressionAST(*x);
    if (*x) guin_mfree(*x);
    *x = NULL;
}




void guin_destroy_VariableDeclarationAST(guin_VariableDeclarationAST* x)
{
    if (!x) return;
    guin_destroy_ExpressionAST_ptr(&x->expression);
    guin_clearstring(&x->info.identifier);
}

void guin_destroy_VariableInfoAST(guin_VariableInfoAST* x)
{
    if (!x) return;
    guin_clearstring(&x->identifier);
    x->datatype = guin_ASTDATATYPE_ERR;
}


guin_ExprFuncCallAST guin_init_ExprFuncCallAST(void)
{
    guin_ExprFuncCallAST x = {0};
    x.expression_args_size = 8;
    x.expression_args = guin_malloc(sizeof(guin_ExpressionAST*) * x.expression_args_size);
    if (!x.expression_args) return (guin_ExprFuncCallAST){0};
    return x;
}
// does not deep copy `toadd`
bool guin_add_ExpressionAST_ptr_to_ExprFuncCallAST(guin_ExprFuncCallAST* x, guin_ExpressionAST* toadd)
{
    if (x->expression_args_length >= x->expression_args_size) {
        size_t original_size = x->expression_args_size;
        while (x->expression_args_length >= x->expression_args_size) 
            x->expression_args_size *= 2;
        guin_ExpressionAST** tmp = guin_remalloc(x->expression_args, sizeof(guin_ExpressionAST*) * x->expression_args_size);
        if (!tmp) {
            x->expression_args_size = original_size;
            return false;
        }
        x->expression_args = tmp;
    }

    x->expression_args[x->expression_args_length++] = toadd;
    return true;
}
void guin_destroy_ExprFuncCallAST(guin_ExprFuncCallAST* x)
{
    if (!x) return;
    if (x->expression_args) {
        for (size_t i = 0; i < x->expression_args_length; ++i)
            guin_destroy_ExpressionAST(x->expression_args[i]);

        guin_mfree(x->expression_args);
        x->expression_args = NULL;
    }
    x->expression_args_length = 0;
}

guin_FunctionAST guin_init_FunctionAST(void)
{
    guin_FunctionAST x = (guin_FunctionAST){0};
    x.args.size = 3;
    x.args.args = guin_malloc(sizeof(guin_FuncArgsAST) * x.args.size);
    if (!x.args.args) return (guin_FunctionAST){0};
    return x;
}
void guin_destroy_FunctionAST(guin_FunctionAST* x)
{
    if (!x) return;
    guin_destroy_ASTScope(&x->scope);
    if (x->args.args) guin_mfree(x->args.args);
    guin_clearstring_ptr(&x->name);
    x->args.length = 0;
    x->args.size   = 0;
}
bool guin_add_arg_to_FunctionAST(guin_FunctionAST* x, guin_ASTDatatype datatype)
{
    if (!x) return false;

    if (x->args.length >= x->args.size) {
        size_t original_size = x->args.size;
        while (x->args.length >= x->args.size) x->args.size *= 2;
        guin_FuncArgsAST* tmp = guin_remalloc(x->args.args, sizeof(guin_FuncArgsAST) * x->args.size);
        if (!tmp) {
            x->args.size = original_size;
            return false;
        }
        x->args.args = tmp;
    }

    x->args.args[x->args.length++].type = datatype;
    return true;
}



guin_ASTScope guin_init_ASTScope(void)
{
    guin_ASTScope x = (guin_ASTScope){0};
    x.size = 6;
    x.nodes = guin_malloc(sizeof(guin_AST) * x.size);
    if (!x.nodes) {
        return (guin_ASTScope){0};
    }
    return x;
}

// does NOT deepcopy pointers in it, just a lightcopy, BEWARE
bool guin_add_AST_to_ASTScope(guin_ASTScope* x, guin_AST* toadd)
{
    if (!x) return false;

    if (x->length >= x->size) {
        size_t original_size = x->size;
        if (x->size > 0) {
            while (x->length >= x->size) x->size *= 2;
        } else {
            x->size = 32;
        }
        guin_AST* tmp = guin_remalloc(x->nodes, x->size * sizeof(guin_AST));
        if (!tmp) {
            x->size = original_size;
            return false;
        }
        x->nodes = tmp;
    }
    x->nodes[x->length++] = *toadd;
    return true;
}

void guin_destroy_ASTScope(guin_ASTScope* x)
{
    if (!x) return;
    if (!x->nodes) return;

    for (size_t i = 0; i < x->length; ++i) {
        guin_destroy_AST(&x->nodes[i]);
    }

    guin_mfree(x->nodes);
    x->nodes  = NULL;
    x->length = 0;
    x->size   = 0;
}



guin_IfWhileAST guin_init_IfWhileAST(void)
{
    guin_IfWhileAST x = (guin_IfWhileAST){0};
    x.nodes = guin_init_ASTScope();
    return x;
}

void guin_destroy_IfWhileAST(guin_IfWhileAST* x)
{
    if (!x) return;
    guin_destroy_ExpressionAST_ptr(&x->expression);
    guin_destroy_ASTScope(&x->nodes);
}



inline guin_AST* guin_init_AST_ptr(guin_ASTNodeType nodetype)
{
    guin_AST* x = guin_malloc(sizeof(guin_AST));
    *x = (guin_AST){0};
    if (!x) return NULL;
    x->nodetype = nodetype;
    return x;
}
void guin_destroy_AST(guin_AST* g_ast)
{
    guin_log("destroying guin_AST...\n");
    switch (g_ast->nodetype)
    {
        case guin_ASTNODE_DECLARATION: { guin_destroy_VariableDeclarationAST(&g_ast->declarationAST);      break; }
        case guin_ASTNODE_EXPRESSION:  { guin_destroy_ExpressionAST(&g_ast->expressionAST);     		   break; }
        case guin_ASTNODE_IF:          { guin_destroy_IfWhileAST(&g_ast->ifWhileAST);                      break; }
        case guin_ASTNODE_WHILE:       { guin_destroy_IfWhileAST(&g_ast->ifWhileAST);                      break; }
        case guin_ASTNODE_SCOPE:       { guin_destroy_ASTScope(&g_ast->scopeAST);                          break; }
        case guin_ASTNODE_RETURN:      { guin_destroy_ExpressionNodeAST_ptr(&g_ast->returnAST.expression); break; }

        case guin_ASTNODE_NULL:                  break;
        case guin_ASTNODE_CLEAR_LOCAL_SLOT:      break;
        case guin_ASTNODE_END:                   break;
        case guin_ASTNODE_IGNORE:                break;
        case guin_ASTNODE_BREAK:                 break;
        case guin_ASTNODE_CONTINUE:              break;
    }
    g_ast->nodetype = guin_ASTNODE_IGNORE;
    guin_log("done destroying guin_AST\n");
}
void guin_destroy_AST_ptr(guin_AST** g_ast)
{
    if (!g_ast) return;
    if (!*g_ast) return;
    guin_destroy_AST(*g_ast);
    guin_mfree(*g_ast);
    *g_ast = NULL;
}

#endif
