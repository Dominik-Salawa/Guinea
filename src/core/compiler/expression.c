#include "ast.h"
#include "parser.h"

#define prev    (pState->prev)
#define current (pState->current)
#define ahead   (pState->ahead)

guin_ExpressionNodeAST*
guin_eval_expression_value_parser(guin_ParseState* pState, const bool in_global_scope)
{
	if (!pState) return NULL;

	guin_ExpressionNodeAST* x = guin_init_ExpressionNodeAST_ptr();
	switch (current.type) 
	{
		case guin_TK_Int_val:
			x->type = guin_EXPRNODE_INT;
			x->data.integer = current.integer;
			break;

		default:
			x->empty = true;
	}
	return x;
}

guin_ExpressionNodeAST*
guin_eval_expression_section_parser(guin_ParseState* pState, const bool in_global_scope)
{
	if (!pState) return NULL;

	guin_ExpressionNodeAST* value = guin_eval_expression_value_parser(pState, in_global_scope);
	if (!value) return NULL;
	if (value->empty) return value;

	if (guin_is_op(ahead.type)) {
		printf("op ahead\n");
		guin_advance_parser(pState);

		guin_ExpressionNodeAST* x = guin_init_ExpressionNodeAST_ptr();
		x->type = guin_LexTokenEnum_to_ValidExpressionNodeType_Operation(current.type);
		x->left = value;

		return x;
	} 
	else return value;
}

guin_ExpressionAST*
guin_eval_expression_parser(guin_ParseState* pState, const bool in_global_scope)
{
	if (!pState) return NULL;

	/*
	
	get first section,

	if its NULL then return NULL
	if its empty, then return an ExpressionAST* with it having NULL as the ->top
	if its a single value then return it finished
	
	if it is an AST tree, keep trying to build the tree until the final value in a single value
	return the tree

	*/
	guin_ExpressionAST* tree = guin_init_ExpressionAST_ptr();
	if (!tree) return NULL;

	guin_ExpressionNodeAST* root = guin_eval_expression_section_parser(pState, in_global_scope);
	if (!root) return NULL;

	if (root->empty) {
		// errorpoint
		guin_destroy_ExpressionNodeAST_ptr(&root);

		guin_ExpressionAST* final = guin_init_ExpressionAST_ptr();
		if (!final) return NULL;

		final->empty = true;
		return final;
	}
	else if (guin_get_pathway_count_of_ExpressionNodeAST(root->type) < 2) {
		guin_ExpressionAST* final = guin_init_ExpressionAST_ptr();
		if (!final) {
			guin_destroy_ExpressionNodeAST_ptr(&root);
			return NULL;
		}

		final->top = root;
		return final;
	} 
	//else fix the tree!!

	// DO NOT TRY TO WIPE current_place, ITS POINTING TO
	// A BRANCH OF root
	guin_ExpressionNodeAST* current_place = root;

	do {
		guin_advance_parser(pState);
		guin_ExpressionNodeAST* chain = guin_eval_expression_section_parser(pState, in_global_scope);
		if (!chain) {
			guin_destroy_ExpressionNodeAST_ptr(&root);

			return NULL;
		}
		if (chain->empty) {
			guin_destroy_ExpressionNodeAST_ptr(&chain);
			// errorpoint
			guin_destroy_ExpressionNodeAST_ptr(&root);
			tree->incomplete = true;
			break;
		}

		int x = guin_get_Precedence_level(current_place->type);
		int y = guin_get_Precedence_level(chain->type);

		if (y == 0 || chain->is_parenthesis) {
			current_place->right = chain;
			break;
		}
		else if (x >= y) {
			current_place->right = chain->left;
			chain->left = root;
			root = current_place = chain;
		} 
		else { // x < y
			current_place->right = chain;
			current_place = chain;
		}
	} while (true);
	
	tree->top = root;
	return tree;
}
