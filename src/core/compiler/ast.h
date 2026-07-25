#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <inttypes.h>
#include "../../../include/declarations.h"
#include "../../etc/strings.h"
#include "../bytecode.h"

typedef struct GUIN_AST GUIN_AST;

typedef enum GUIN_ASTDatatype {
    GUIN_ASTDATATYPE_ERR = 0, // FOR ERRORS
    GUIN_ASTDATATYPE_VOID,
    GUIN_ASTDATATYPE_NIL,
    GUIN_ASTDATATYPE_INT,
    GUIN_ASTDATATYPE_NUMBER,
    GUIN_ASTDATATYPE_BOOL,
    GUIN_ASTDATATYPE_FUNCTION,
    GUIN_ASTDATATYPE_STRING,
    GUIN_ASTDATATYPE_CHAR,
    GUIN_ASTDATATYPE_DYNAMIC
} GUIN_ASTDatatype;
char* GUIN_ASTDatatype_to_string(GUIN_ASTDatatype dt);


typedef enum GUIN_ExpressionNodeType {
    GUIN_EXPRNODE_UNINIT = 0,
    GUIN_EXPRNODE_PARENTHESIS,

    // OPERATION
    GUIN_EXPRNODE_ADD,
    GUIN_EXPRNODE_SUB,
    GUIN_EXPRNODE_MUL,
    GUIN_EXPRNODE_DIV,
    GUIN_EXPRNODE_POW,
    GUIN_EXPRNODE_MOD,
    GUIN_EXPRNODE_NEG,  // make stuff negative, only on the right side it continues (SHOULD ONLY BE FOR VARIABLES!!! NOT NUMBERS OR INTS)
    GUIN_EXPRNODE_CALL, // func call, the right side is where it continues

    // MORE OPERATIONS
    GUIN_EXPRNODE_EQU,
    GUIN_EXPRNODE_NOT_EQU,
    GUIN_EXPRNODE_GT,
    GUIN_EXPRNODE_LT,
    GUIN_EXPRNODE_GT_EQU,
    GUIN_EXPRNODE_LT_EQU,

    GUIN_EXPRNODE_NOT,
    GUIN_EXPRNODE_AND,
    GUIN_EXPRNODE_OR,

    // DATATYPES
    GUIN_EXPRNODE_INT,
    GUIN_EXPRNODE_NUMBER,
    GUIN_EXPRNODE_STRING,
    GUIN_EXPRNODE_BOOL,
    GUIN_EXPRNODE_CHAR,
    GUIN_EXPRNODE_FUNCTION_LITERAL,
    GUIN_EXPRNODE_GLOBAL_IDENTIFIER,
    GUIN_EXPRNODE_LOCAL_IDENTIFIER,
    GUIN_EXPRNODE_NIL
} GUIN_ExpressionNodeType;
GUIN_byte GUIN_get_pathway_count_of_ExpressionNodeAST(GUIN_ExpressionNodeType type);
char* GUIN_ExpressionNodeType_to_string(GUIN_ExpressionNodeType dt);


typedef struct GUIN_ExpressionAST GUIN_ExpressionAST;

typedef struct GUIN_ExprFuncCallAST {
    GUIN_ExpressionAST** expression_args;
    size_t expression_args_length;
    size_t expression_args_size;
} GUIN_ExprFuncCallAST;
GUIN_ExprFuncCallAST GUIN_init_ExprFuncCallAST(void);
bool GUIN_add_ExpressionAST_ptr_to_ExprFuncCallAST(GUIN_ExprFuncCallAST* x, GUIN_ExpressionAST* toadd);
void GUIN_destroy_ExprFuncCallAST(GUIN_ExprFuncCallAST* x);



typedef struct GUIN_FuncArgsAST {
    GUIN_ASTDatatype type;
} GUIN_FuncArgsAST;

typedef struct FunctionAST {
    GUIN_Bytecode bytecode;
    GUIN_ASTDatatype return_type;

    struct {
        GUIN_FuncArgsAST* args;
        size_t size;
        size_t length;
    } args;
} GUIN_FunctionAST;
void GUIN_destroy_FunctionAST(GUIN_FunctionAST* x);



typedef struct GUIN_ExpressionNodeAST {
    struct GUIN_ExpressionNodeAST* left;
    struct GUIN_ExpressionNodeAST* right;
    GUIN_ExpressionNodeType type;

    struct {
        size_t line;
        size_t column;
        size_t length;
    } info;

    union {
        GUIN_number64 number;
        GUIN_int64  integer;
        GUIN_String string_identifier;
        GUIN_LOCAL_SLOT_INT slot_num;
        char ch;
        bool bl;
        GUIN_FunctionAST     function;
        GUIN_ExprFuncCallAST exprFuncCallAST;
    } data;
} GUIN_ExpressionNodeAST;
bool GUIN_assign_ExpressionNodeAST(GUIN_ExpressionNodeAST** x, GUIN_ExpressionNodeType type);
void GUIN_destroy_ExpressionNodeAST_ptr(GUIN_ExpressionNodeAST** x);

typedef struct GUIN_ExpressionAST {
    bool fail;
    GUIN_ExpressionNodeAST* top;
} GUIN_ExpressionAST;
void GUIN_destroy_ExpressionAST(GUIN_ExpressionAST* x);
void GUIN_destroy_ExpressionAST_ptr(GUIN_ExpressionAST** x);


typedef struct GUIN_VariableInfoAST {
    GUIN_String identifier; // the name attached to this variable
    //ubyte structure_type; // the structure of this variable
    //bool is_ptr;
    bool allowed_in_global_expression;
    GUIN_LOCAL_SLOT_INT slot;

    union {
        GUIN_ASTDatatype datatype; // for single datatype
        //ubyte* datatypes; // for multiple datatypes
        //size_t data_types_size; // size of multiple datatypes
    };
} GUIN_VariableInfoAST;
void GUIN_destroy_VariableInfoAST(GUIN_VariableInfoAST* x);


typedef struct GUIN_VariableDeclarationAST {
    GUIN_VariableInfoAST info;
    GUIN_ExpressionAST* expression;
    GUIN_LOCAL_SLOT_INT slot;
} GUIN_VariableDeclarationAST;
void GUIN_destroy_VariableDeclarationAST(GUIN_VariableDeclarationAST* x);






typedef struct GUIN_ASTScope {
    size_t size;
    size_t length;
    GUIN_AST* nodes;
} GUIN_ASTScope;

GUIN_ASTScope GUIN_init_ASTScope(void);
// does NOT deepcopy pointers in it, just a lightcopy, BEWARE
bool GUIN_add_AST_to_ASTScope(GUIN_ASTScope* x, GUIN_AST toadd);
void GUIN_destroy_ASTScope(GUIN_ASTScope* x);


// USED TO REPRESENT BOTH IF AND WHILE AST
typedef struct GUIN_IfWhileAST {
    GUIN_ASTScope nodes;
    GUIN_ExpressionAST* expression;
} GUIN_IfWhileAST;
GUIN_IfWhileAST GUIN_init_IfWhileAST(void);
void GUIN_destroy_IfWhileAST(GUIN_IfWhileAST* x);


typedef struct AssignAST {
    GUIN_ExpressionNodeAST* top;
    GUIN_ExpressionAST* assignment;
} GUIN_AssignAST;


typedef struct GUIN_ClearLocalSlotAST {
    GUIN_LOCAL_SLOT_INT slot;
} GUIN_ClearLocalSlotAST;


typedef enum GUIN_ASTNodeType {
    GUIN_ASTNODE_IGNORE = 0,  // FOR THE IR TO SIMPLY IGNORE
    GUIN_ASTNODE_END,         // PARSER HAS REACHED THE END of its desired token (like end for a function or EOF for a file)
    GUIN_ASTNODE_CLEAR_LOCAL_SLOT,

    GUIN_ASTNODE_DECLARATION,
    GUIN_ASTNODE_ASSIGN,
    GUIN_ASTNODE_IF,
    GUIN_ASTNODE_WHILE,
    GUIN_ASTNODE_SCOPE,
} GUIN_ASTNodeType;

typedef struct GUIN_AST {
    GUIN_ASTNodeType nodetype;
    bool error;

    union {
        GUIN_VariableDeclarationAST declarationAST;
        GUIN_IfWhileAST             ifWhileAST;
        GUIN_ASTScope               scopeAST;
        GUIN_AssignAST              assignAST;
        GUIN_ClearLocalSlotAST      clearLocalSlotAST;
    };
} GUIN_AST;
void GUIN_destroy_AST(GUIN_AST* g_ast);

#endif
