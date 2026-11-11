#ifndef guin_PARSER_c
#define guin_PARSER_c

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../etc/strings.h"
#include "../../etc/log.h"
#include "lexer.h"
#include "parser.h"
#include "parse-state.h"
#include "ast.h"
#include "expressions.h"

static guin_LexTokenEnum valid_Operations[] = {
    guin_TK_ADD,
    guin_TK_SUB,
    guin_TK_MUL,
    guin_TK_DIV,
    guin_TK_MOD,
    guin_TK_POW,

    guin_TK_and,
    guin_TK_or,

    guin_TK_EQU,
    guin_TK_NOT_EQU,
    guin_TK_GT,
    guin_TK_LT,
    guin_TK_GT_EQU,
    guin_TK_LT_EQU,
	guin_TK_ASSIGN,
};

struct guin_ExpressionPrecedenceStruct {
    guin_ubyte rank;
    guin_ExpressionNodeType type;
};

// 0 == (literal value like int or string)
static struct guin_ExpressionPrecedenceStruct exprNodeOps[] = {
    { .rank=1,.type=guin_EXPRNODE_ASSIGN  },
    
	{ .rank=2,.type=guin_EXPRNODE_AND     },

    { .rank=3,.type=guin_EXPRNODE_OR      },
    
    { .rank=4,.type=guin_EXPRNODE_EQU     },
    { .rank=4,.type=guin_EXPRNODE_NOT_EQU },
    { .rank=4,.type=guin_EXPRNODE_GT      },
    { .rank=4,.type=guin_EXPRNODE_LT      },
    { .rank=4,.type=guin_EXPRNODE_GT_EQU  },
    { .rank=4,.type=guin_EXPRNODE_LT_EQU  },

    { .rank=5,.type=guin_EXPRNODE_ADD     },
    { .rank=5,.type=guin_EXPRNODE_SUB     },

    { .rank=6,.type=guin_EXPRNODE_MUL     },
    { .rank=6,.type=guin_EXPRNODE_DIV     },
    { .rank=6,.type=guin_EXPRNODE_MOD     },

    { .rank=7,.type=guin_EXPRNODE_POW     },

};

guin_ExpressionNodeType guin_LexTokenEnum_to_ValidExpressionNodeType_Operation(guin_LexTokenEnum x)
{
    switch (x)
    {
        case guin_TK_Number_val:     return guin_EXPRNODE_NUMBER;
        case guin_TK_Int_val:        return guin_EXPRNODE_INT;
        case guin_TK_String_val:     return guin_EXPRNODE_STRING;
        case guin_TK_Char_val:       return guin_EXPRNODE_CHAR;
        case guin_TK_Bool_val:       return guin_EXPRNODE_BOOL;

        case guin_TK_ADD:            return guin_EXPRNODE_ADD;
        case guin_TK_SUB:            return guin_EXPRNODE_SUB;
        case guin_TK_MUL:            return guin_EXPRNODE_MUL;
        case guin_TK_DIV:            return guin_EXPRNODE_DIV;
        case guin_TK_MOD:            return guin_EXPRNODE_MOD;
        case guin_TK_POW:            return guin_EXPRNODE_POW;

        case guin_TK_and:            return guin_EXPRNODE_AND;
        case guin_TK_or:             return guin_EXPRNODE_OR;

        case guin_TK_GT:             return guin_EXPRNODE_GT;
        case guin_TK_LT:             return guin_EXPRNODE_LT;
        case guin_TK_GT_EQU:         return guin_EXPRNODE_GT_EQU;
        case guin_TK_LT_EQU:         return guin_EXPRNODE_LT_EQU;
        case guin_TK_EQU:            return guin_EXPRNODE_EQU;
        case guin_TK_NOT_EQU:        return guin_EXPRNODE_NOT_EQU;
		case guin_TK_ASSIGN:		 return guin_EXPRNODE_ASSIGN;
		default: return guin_EXPRNODE_UNINIT;
    }
}

// 0 == (literal value like int or string)
guin_ubyte guin_get_Precedence_level(guin_ExpressionNodeType type)
{
    for (size_t i = 0; i < sizeof(exprNodeOps)/sizeof(struct guin_ExpressionPrecedenceStruct); i++)
        if (exprNodeOps[i].type == type) return exprNodeOps[i].rank;
    return 0;
}

#define guin_pState_prev    (pState->prev)
#define guin_pState_current (pState->current)
#define guin_pState_ahead   (pState->ahead)

guin_AST* guin_eval_variable_parser(guin_ParseState* pState, const bool in_global_scope)
{
	return NULL;
}
guin_AST* guin_eval_namespace_parser(guin_ParseState* pState, const bool in_global_scope)
{
	return NULL;
}

guin_AST* guin_parse_segment(guin_ParseState* pState, const bool in_global_scope)
{
	guin_advance_parser(pState);

	switch (guin_pState_current.type)
	{
		case guin_TK_var:
			return guin_eval_variable_parser(pState, in_global_scope);
		case guin_TK_namespace:
			return guin_eval_namespace_parser(pState, in_global_scope);

		default: {
			guin_AST* x = guin_init_AST_ptr(guin_ASTNODE_EXPRESSION);
			if (!x) return NULL;

			guin_ExpressionAST* expr = guin_eval_expression_parser(pState, in_global_scope);
			if (!expr) {
				guin_destroy_AST_ptr(&x);
				return NULL;
			}
			else if (expr->empty) {
				pState->errmsg = "Failed to fetch a statement!";
				x->fail = true;
				return x;
			}
			else if (expr->incomplete) {
				pState->errmsg = "Expression was left incomplete!";
				x->fail = true;
				return x;
			}
			else if (expr->fail) {
				x->fail = true;
				return x;
			}

			x->expressionAST = *expr;
			guin_mfree(expr);
			return x;
		}
	}
}

#endif
