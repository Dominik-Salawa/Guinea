static ExpressionAST* eval_expression_parser(ParseState* pState, LexTokenEnum token_to_signify_end, bool is_global_scope)
{
    ExpressionAST* root_exprAST = init_ExpressionAST_ptr();
    ExpressionNodeAST** current_exprASTNode = &root_exprAST->top;
    
    ExpressionAST* left_exprAST  = NULL;
    ExpressionAST* right_exprAST = NULL;
    while (true) {
        G_log("fetching left\n");
        G_log_push_layer();
        left_exprAST = get_op_expression_parser(pState, token_to_signify_end, is_global_scope);
        G_log_pop_layer();

        if (!left_exprAST) {
            G_log("FATAL: left doesnt exist!\n");
            destroy_ExpressionAST_ptr(&root_exprAST);
            break;
        }
        if (left_exprAST->fail) {
            G_log("failed from get_op_expression_parser!\n");
            destroy_ExpressionNodeAST_ptr(&root_exprAST->top);
            root_exprAST->fail = true;
            break;
        }
        if (!left_exprAST->top) {
            G_log("end!\n");
            break;
        }

        if (!root_exprAST->top) {
            root_exprAST->top = left_exprAST->top;
        }

        G_log("fetching right\n");
        G_log_push_layer();
        ExpressionAST* right_exprAST = get_op_expression_parser(pState, token_to_signify_end, is_global_scope);
        G_log_pop_layer();

        if (!right_exprAST) {
            G_log("FATAL: right doesnt exist!\n");
            destroy_ExpressionAST_ptr(&root_exprAST);
            break;
        }

        G_log("CUR:%s\n", ExpressionNodeType_to_string(left_exprAST->top->type));
        { // Check if right_exprAST is valid
            if (right_exprAST->fail) {
                root_exprAST->fail = true;
                destroy_ExpressionAST_ptr(&right_exprAST);
                destroy_ExpressionNodeAST_ptr(&root_exprAST->top);
                break;
            }

            if (!right_exprAST->top) {
                root_exprAST->fail = true;
                pState->errmsg = "Expected a valid value after the operation!";
                destroy_ExpressionAST_ptr(&right_exprAST);
                destroy_ExpressionNodeAST_ptr(&root_exprAST->top);
                break;
            }
        }



        { // Checking who has greater precedence
            G_log("checking precedence...\n");
            byte left_exprAST_precedence    = get_Precedence_level(left_exprAST->top->type);
            byte right_exprAST_precedence   = get_Precedence_level(right_exprAST->top->type);

                                                    // 0 == single val
            if (right_exprAST_precedence < left_exprAST_precedence && right_exprAST_precedence != 0) {
                G_log("left higher precedence\n");

                left_exprAST->top->right = right_exprAST->top->left;
                right_exprAST->top->left = left_exprAST->top;

                // now exprAST inherits tree and we scrap tree
                left_exprAST->top = right_exprAST->top;

                *current_exprASTNode = left_exprAST->top;
                current_exprASTNode  = &(*current_exprASTNode)->right;
            }
            else if (right_exprAST_precedence == left_exprAST_precedence) {
                G_log("equal precedence\n");
                printf("%s %s\n", ExpressionNodeType_to_string(left_exprAST->top->type), ExpressionNodeType_to_string(right_exprAST->top->type));
                exit(1);
                //exprAST->top->right = tree->top;
                //exprAST->top->left = tree->top->left;
                //tree->top->left = exprAST->top;

                // now exprAST inherits tree and we scrap tree
                //exprAST->top = tree->top;
            }
            else { // tree_precedence > exprAST_precedence || tree_precedence == 0
                G_log("right higher precedence\n");
                left_exprAST->top->right = right_exprAST->top;
                *current_exprASTNode = left_exprAST->top;
                current_exprASTNode  = &(*current_exprASTNode)->left;
            }
        }
        
        // Delete tree
        right_exprAST->top = NULL;
        destroy_ExpressionAST_ptr(&right_exprAST);

        left_exprAST->top = NULL;
        destroy_ExpressionAST_ptr(&left_exprAST);
    }

    destroy_ExpressionAST_ptr(&left_exprAST);
    destroy_ExpressionAST_ptr(&right_exprAST);
    return root_exprAST;
}