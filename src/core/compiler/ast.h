#ifndef guin_AST_h
#define guin_AST_h

#include <stdlib.h>
#include <inttypes.h>
#include "../../../include/declarations.h"
#include "../../etc/strings.h"

typedef struct guin_AST guin_AST;
typedef struct guin_ASTScope guin_ASTScope;

typedef enum guin_ASTDatatype {
    guin_ASTDATATYPE_ERR = 0, // FOR ERRORS
    guin_ASTDATATYPE_VOID,
    guin_ASTDATATYPE_NIL,
    guin_ASTDATATYPE_INT,
    guin_ASTDATATYPE_NUMBER,
    guin_ASTDATATYPE_BOOL,
    guin_ASTDATATYPE_FUNCTION,
    guin_ASTDATATYPE_STRING,
    guin_ASTDATATYPE_CHAR,
    guin_ASTDATATYPE_DYNAMIC
} guin_ASTDatatype;
char* guin_ASTDatatype_to_string(guin_ASTDatatype dt);


typedef enum guin_ExpressionNodeType {
    guin_EXPRNODE_UNINIT = 0,

    // OPERATION
    guin_EXPRNODE_ADD,
    guin_EXPRNODE_SUB,
    guin_EXPRNODE_MUL,
    guin_EXPRNODE_DIV,
    guin_EXPRNODE_POW,
    guin_EXPRNODE_MOD,
    guin_EXPRNODE_NEG,    // make stuff negative, only on the right side it continues (SHOULD ONLY BE FOR VARIABLES!!! NOT NUMBERS OR INTS)
	guin_EXPRNODE_ASSIGN, // left is to what to be assigned to, right is to what it should be assigned with 
	guin_EXPRNODE_CALL,   // func call, the right side is where it continues

    // MORE OPERATIONS
    guin_EXPRNODE_EQU,
    guin_EXPRNODE_NOT_EQU,
    guin_EXPRNODE_GT,
    guin_EXPRNODE_LT,
    guin_EXPRNODE_GT_EQU,
    guin_EXPRNODE_LT_EQU,

    guin_EXPRNODE_NOT,
    guin_EXPRNODE_AND,
    guin_EXPRNODE_OR,

    // DATATYPES
    guin_EXPRNODE_INT,
    guin_EXPRNODE_NUMBER,
    guin_EXPRNODE_STRING,
    guin_EXPRNODE_BOOL,
    guin_EXPRNODE_CHAR,
    guin_EXPRNODE_FUNCTION_LITERAL,
    guin_EXPRNODE_GLOBAL_IDENTIFIER,
    guin_EXPRNODE_LOCAL_IDENTIFIER,
    guin_EXPRNODE_NIL,
} guin_ExpressionNodeType;
int guin_get_pathway_count_of_ExpressionNodeAST(guin_ExpressionNodeType type);
char* guin_ExpressionNodeType_to_string(guin_ExpressionNodeType dt);


typedef struct guin_ASTScope {
    size_t size;
    size_t length;
    guin_AST* nodes;
    bool error;
} guin_ASTScope;

guin_ASTScope guin_init_ASTScope(void);
// does NOT deepcopy pointers in it, just a lightcopy, BEWARE
bool guin_add_AST_to_ASTScope(guin_ASTScope* x, guin_AST* toadd);
void guin_destroy_ASTScope(guin_ASTScope* x);


typedef struct guin_ExpressionAST guin_ExpressionAST;

typedef struct guin_ExprFuncCallAST {
    guin_ExpressionAST** expression_args;
    size_t expression_args_length;
    size_t expression_args_size;
} guin_ExprFuncCallAST;
guin_ExprFuncCallAST guin_init_ExprFuncCallAST(void);
bool guin_add_ExpressionAST_ptr_to_ExprFuncCallAST(guin_ExprFuncCallAST* x, guin_ExpressionAST* toadd);
void guin_destroy_ExprFuncCallAST(guin_ExprFuncCallAST* x);


typedef struct guin_FuncArgsAST {
    guin_ASTDatatype type;
} guin_FuncArgsAST;

typedef struct FunctionAST {
    guin_ASTScope scope;
    guin_ASTDatatype return_type;
    guin_String* name;
    struct {
        guin_FuncArgsAST* args;
        size_t size;
        size_t length;
    } args;
} guin_FunctionAST;
guin_FunctionAST guin_init_FunctionAST(void);
void guin_destroy_FunctionAST(guin_FunctionAST* x);


typedef struct guin_ExpressionNodeAST {
    struct guin_ExpressionNodeAST* left;
    struct guin_ExpressionNodeAST* right;
    guin_ExpressionNodeType type;

	bool empty;
	bool is_parenthesis;

	/*
    struct {
        size_t line;
        size_t column;
        size_t length;
    } info;
	*/

    union {
        guin_number64 number;
        guin_int64  integer;
        guin_String string_identifier;
        guin_LOCAL_SLOT_INT slot_num;
        char ch;
        bool bl;
        guin_FunctionAST     function;
        guin_ExprFuncCallAST exprFuncCallAST;
    } data;
} guin_ExpressionNodeAST;
guin_ExpressionNodeAST* guin_init_ExpressionNodeAST_ptr(void);
bool guin_assign_ExpressionNodeAST(guin_ExpressionNodeAST** x, guin_ExpressionNodeType type);
void guin_destroy_ExpressionNodeAST(guin_ExpressionNodeAST* x);
void guin_destroy_ExpressionNodeAST_ptr(guin_ExpressionNodeAST** x);

typedef struct guin_ExpressionAST {
    bool incomplete;
    bool empty; // there was no tree to begin with
    bool is_parenthesis; // for the function guin_eval_expression_parser so it can correctly identify instead of mistaking for wrong precendence
    guin_ExpressionNodeAST* top;
} guin_ExpressionAST;
guin_ExpressionAST* guin_init_ExpressionAST_ptr(void);
void guin_destroy_ExpressionAST(guin_ExpressionAST* x);
void guin_destroy_ExpressionAST_ptr(guin_ExpressionAST** x);


typedef struct guin_VariableInfoAST {
    guin_String identifier; // the name attached to this variable
    //ubyte structure_type; // the structure of this variable
    //bool is_ptr;
    // this is for whether the variable is allowed to be used in a global expression
    bool allowed_in_global_expression;
    guin_LOCAL_SLOT_INT slot;

    union {
        guin_ASTDatatype datatype; // for single datatype
        //ubyte* datatypes; // for multiple datatypes
        //size_t data_types_size; // size of multiple datatypes
    };
} guin_VariableInfoAST;
void guin_destroy_VariableInfoAST(guin_VariableInfoAST* x);


typedef struct guin_VariableDeclarationAST {
    guin_VariableInfoAST info;
    guin_ExpressionAST* expression;
    guin_LOCAL_SLOT_INT slot;
} guin_VariableDeclarationAST;
void guin_destroy_VariableDeclarationAST(guin_VariableDeclarationAST* x);







// USED TO REPRESENT BOTH IF AND WHILE AST
typedef struct guin_IfWhileAST {
    guin_ASTScope nodes;
    guin_ExpressionAST* expression;
} guin_IfWhileAST;
guin_IfWhileAST guin_init_IfWhileAST(void);
void guin_destroy_IfWhileAST(guin_IfWhileAST* x);


typedef struct guin_ClearLocalSlotAST {
    guin_LOCAL_SLOT_INT slot;
} guin_ClearLocalSlotAST;


typedef struct guin_ReturnAST {
    guin_ExpressionNodeAST* expression;
} guin_ReturnAST;


typedef enum guin_ASTNodeType {
    guin_ASTNODE_NULL = 0,
    guin_ASTNODE_IGNORE,  // FOR THE IR TO SIMPLY IGNORE
    guin_ASTNODE_END,         // PARSER HAS REACHED THE END of its desired token (like end for a function or EOF for a file)
    guin_ASTNODE_CLEAR_LOCAL_SLOT,

    guin_ASTNODE_DECLARATION,
    guin_ASTNODE_EXPRESSION,
    guin_ASTNODE_IF,
    guin_ASTNODE_WHILE,
    guin_ASTNODE_SCOPE,

    guin_ASTNODE_BREAK,
    guin_ASTNODE_CONTINUE,
    guin_ASTNODE_RETURN,
} guin_ASTNodeType;

typedef struct guin_AST {
    guin_ASTNodeType nodetype;
    bool error;

    union {
        guin_VariableDeclarationAST declarationAST;
		guin_ExpressionAST			expressionAST;
        guin_IfWhileAST             ifWhileAST;
        guin_ASTScope               scopeAST;
        guin_ClearLocalSlotAST      clearLocalSlotAST;
        guin_ReturnAST              returnAST;
    };
} guin_AST;
guin_AST* guin_init_AST_ptr(guin_ASTNodeType nodetype);
void guin_destroy_AST(guin_AST* g_ast);

#endif
