#pragma once

#include <stdbool.h>

#include "lexer.h"
#include "string_view.h"
#include "vector.h"

typedef enum {
    EXP_CONSTANT,
    EXP_VAR,
    EXP_CONDITIONAL,
    EXP_UNARY_COMPLEMENT,
    EXP_UNARY_NEG,
    EXP_UNARY_LOGICAL_NOT,
    EXP_UNARY_PRE_INCREMENT,
    EXP_UNARY_POST_INCREMENT,
    EXP_UNARY_PRE_DECREMENT,
    EXP_UNARY_POST_DECREMENT,
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
    EXP_BINARY_LOGICAL_AND,
    EXP_BINARY_LOGICAL_OR,
    EXP_BINARY_ASSIGN,
    EXP_BINARY_ADD_ASSIGN,
    EXP_BINARY_SUB_ASSIGN,
    EXP_BINARY_MUL_ASSIGN,
    EXP_BINARY_DIV_ASSIGN,
    EXP_BINARY_REMAINDER_ASSIGN,
    EXP_BINARY_AND_ASSIGN,
    EXP_BINARY_OR_ASSIGN,
    EXP_BINARY_XOR_ASSIGN,
    EXP_BINARY_LEFT_SHIFT_ASSIGN,
    EXP_BINARY_RIGHT_SHIFT_ASSIGN,
    EXP_BINARY_EQUAL,
    EXP_BINARY_NOT_EQUAL,
    EXP_BINARY_LESS_THAN,
    EXP_BINARY_GREATER_THAN,
    EXP_BINARY_LESS_OR_EQUAL,
    EXP_BINARY_GREATER_OR_EQUAL,
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
        StringView var_str;
        unsigned var_id;
        struct {
            AST_Expression *left_exp;
            AST_Expression *right_exp;
        };
        struct {
            AST_Expression *cond;
            AST_Expression *true_exp;
            AST_Expression *false_exp;
        };
    };
};

typedef enum {
    AST_STATEMENT_RETURN,
    AST_STATEMENT_EXP,
    AST_STATEMENT_NULL,
    AST_STATEMENT_IF,
    AST_STATEMENT_IF_ELSE,
    AST_DECLARATION_NO_ASSIGN,
    AST_DECLARATION_WITH_ASSIGN,
} AST_BlockItemType;
typedef struct AST_BlockItem AST_BlockItem;
struct AST_BlockItem {
    AST_BlockItemType type;
    union {
        AST_Expression exp;
        struct {
            AST_Expression var;
            AST_Expression assign_exp;
        };
        struct {
            AST_Expression cond;
            AST_BlockItem *then;
            AST_BlockItem *else_;
        };
    };
};
vector_header(AST_BlockItem);

typedef struct {
    AST_Identifier name;
    VectorAST_BlockItem block_items;
} AST_Function;

typedef struct {
    AST_Function function;
    int error;
    unsigned var_cnt;
} AST_Program;

AST_Program parse_program(VectorTerm _terms);
void pretty_print_program(AST_Program program, bool variable_resolved);
