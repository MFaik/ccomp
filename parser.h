#pragma once

#include "lexer.h"
#include "string_view.h"

typedef enum {
    STATEMENT_RETURN,
    STATEMENT_IF,
} AST_StatementType;
typedef enum {
    EXP_CONSTANT,
    EXP_UNARY_COMPLEMENT,
    EXP_UNARY_NEG,
    EXP_BINARY_ADD,
    EXP_BINARY_SUB,
    EXP_BINARY_MUL,
    EXP_BINARY_DIV,
    EXP_BINARY_REMAINDER,
    EXP_BINARY_LEFT_SHIFT,
    EXP_BINARY_RIGHT_SHIFT,
    EXP_BINARY_BITWISE_AND,
    EXP_BINARY_BITWISE_OR,
    EXP_BINARY_BITWISE_XOR,
} AST_ExpressionType;

typedef struct {
    StringView str;
} AST_Identifier;

typedef struct AST_Expression AST_Expression;
struct AST_Expression {
    AST_ExpressionType type;
    union {
        int constant;
        AST_Expression *unary_exp;
        struct {
            AST_Expression *left_exp;
            AST_Expression *right_exp;
        };
    };
};

typedef struct {
    AST_Expression ret;
} AST_Statement;

typedef struct {
    AST_Identifier name;
    AST_Statement statement;
} AST_Function;

typedef struct {
    AST_Function function;
    int error;
} AST_Program;

AST_Program parse_program(VectorTerm _terms);
void pretty_print_program(AST_Program program);

