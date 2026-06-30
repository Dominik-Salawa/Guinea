#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <inttypes.h>
#include "../../etc/declarations.h"
#include "../../etc/strings.h"
#include "../bytecode.h"

typedef struct G_AST G_AST;

typedef enum ASTDatatype {
    ASTDATATYPE_ERR = 0, // FOR ERRORS
    ASTDATATYPE_NIL,
    ASTDATATYPE_INT,
    ASTDATATYPE_NUMBER,
    ASTDATATYPE_BOOL,
    ASTDATATYPE_FUNCTION,
    ASTDATATYPE_STRING,
    ASTDATATYPE_CHAR,
    ASTDATATYPE_DYNAMIC
} ASTDatatype;
char* Datatype_to_string(ASTDatatype dt)
{
    switch (dt)
    {
        case ASTDATATYPE_ERR:          return "err";
        case ASTDATATYPE_NIL:          return "nil";
        case ASTDATATYPE_INT:          return "int";
        case ASTDATATYPE_NUMBER:       return "number";
        case ASTDATATYPE_BOOL:         return "bool";
        case ASTDATATYPE_FUNCTION:     return "function";
        case ASTDATATYPE_STRING:       return "string";
        case ASTDATATYPE_CHAR:         return "char";
        case ASTDATATYPE_DYNAMIC:      return "dynamic";
    }
    return NULL;
}

typedef enum ExpressionNodeType {
    EXPRNODE_UNINIT = 0,
    EXPRNODE_PARENTHESIS,

    // OPERATION
    EXPRNODE_ADD,
    EXPRNODE_SUB,
    EXPRNODE_MUL,
    EXPRNODE_DIV,
    EXPRNODE_POW,
    EXPRNODE_MOD,
    EXPRNODE_NEG,  // make stuff negative, only on the right side it continues (SHOULD ONLY BE FOR VARIABLES!!! NOT NUMBERS OR INTS)
    EXPRNODE_CALL, // func call, the right side is where it continues

    // MORE OPERATIONS
    EXPRNODE_EQU,
    EXPRNODE_NOT_EQU,
    EXPRNODE_GT,
    EXPRNODE_LT,
    EXPRNODE_GT_EQU,
    EXPRNODE_LT_EQU,

    EXPRNODE_NOT,
    EXPRNODE_AND,
    EXPRNODE_OR,

    // DATATYPES
    EXPRNODE_INT,
    EXPRNODE_NUMBER,
    EXPRNODE_STRING,
    EXPRNODE_BOOL,
    EXPRNODE_CHAR,
    EXPRNODE_FUNCTION_LITERAL,
    EXPRNODE_IDENTIFIER,
    EXPRNODE_NIL
} ExpressionNodeType;
G_byte get_pathway_count_of_ExpressionNodeAST(ExpressionNodeType type);
char* ExpressionNodeType_to_string(ExpressionNodeType dt);


typedef struct ExpressionAST ExpressionAST;

typedef struct ExprFuncCallAST {
    ExpressionAST* expression_args;
    size_t expression_args_length;
} ExprFuncCallAST;
void destroy_ExprFuncCallAST(ExprFuncCallAST* x);



typedef struct FuncArgsAST {
    ASTDatatype type;
} FuncArgsAST;

typedef struct FunctionAST {
    G_Bytecode bytecode;
    ASTDatatype return_type;

    struct {
        FuncArgsAST* args;
        size_t size;
        size_t length;
    } args;
} FunctionAST;
void destroy_FunctionAST(FunctionAST* x);



typedef struct ExpressionNodeAST {
    struct ExpressionNodeAST* left;
    struct ExpressionNodeAST* right;
    ExpressionNodeType type;

    struct {
        size_t line;
        size_t column;
        size_t length;
    } info;

    union {
        G_number64 number;
        G_int64  integer;
        String string_identifier;
        char ch;
        bool bl;
        FunctionAST     function;
        ExprFuncCallAST exprFuncCallAST;
    } data;
} ExpressionNodeAST;
bool assign_ExpressionNodeAST(ExpressionNodeAST** x, ExpressionNodeType type);
void destroy_ExpressionNodeAST(ExpressionNodeAST* x);
void destroy_ExpressionNodeAST_ptr(ExpressionNodeAST** x);

typedef struct ExpressionAST {
    bool fail;
    ExpressionNodeAST* top;
} ExpressionAST;
void destroy_ExpressionAST(ExpressionAST* x);
void destroy_ExpressionAST_ptr(ExpressionAST** x);


typedef struct VariableInfoAST {
    String identifier; // the name attached to this variable
    //ubyte structure_type; // the structure of this variable
    //bool is_ptr;
    G_LOCAL_SLOT_INT slot;

    union {
        ASTDatatype datatype; // for single datatype
        //ubyte* datatypes; // for multiple datatypes
        //size_t data_types_size; // size of multiple datatypes
    };
} VariableInfoAST;
void destroy_VariableInfoAST(VariableInfoAST* x);


typedef struct VariableDeclarationAST {
    VariableInfoAST info;
    ExpressionAST* expression;
    G_LOCAL_SLOT_INT slot;
} VariableDeclarationAST;
void destroy_VariableDeclarationAST(VariableDeclarationAST* x);






typedef struct ASTScope {
    size_t size;
    size_t length;
    G_AST* nodes;
} ASTScope;

ASTScope init_ASTScope();
// does NOT deepcopy pointers in it, just a lightcopy, BEWARE
bool add_G_AST_to_ASTScope(ASTScope* x, G_AST toadd);
void destroy_ASTScope(ASTScope* x);


// USED TO REPRESENT BOTH IF AND WHILE AST
typedef struct IfWhileAST {
    ASTScope nodes;
    ExpressionAST* expression;
} IfWhileAST;
IfWhileAST init_IfWhileAST();
void destroy_IfWhileAST(IfWhileAST* x);


typedef struct AssignAST {
    ExpressionNodeAST* top;
    ExpressionAST* assignment;
} AssignAST;


typedef struct ClearLocalSlotAST {
    G_LOCAL_SLOT_INT slot;
} ClearLocalSlotAST;


typedef enum ASTNodeType {
    ASTNODE_IGNORE = 0,  // FOR THE IR TO SIMPLY IGNORE
    ASTNODE_END,         // PARSER HAS REACHED THE END of its desired token (like end for a function or EOF for a file)
    ASTNODE_CLEAR_LOCAL_SLOT,

    ASTNODE_DECLARATION,
    ASTNODE_ASSIGN,
    ASTNODE_IF,
    ASTNODE_WHILE,
    ASTNODE_SCOPE
} ASTNodeType;

typedef struct G_AST {
    ASTNodeType nodetype;
    bool error;

    union {
        VariableDeclarationAST declarationAST;
        IfWhileAST             ifWhileAST;
        ASTScope               scopeAST;
        AssignAST              assignAST;
        ClearLocalSlotAST      clearLocalSlotAST;
    };
} G_AST;
void destroy_G_AST(G_AST* g_ast);

#endif