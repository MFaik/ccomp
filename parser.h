#pragma once

#include "lexer.h"
#include "string_view.h"

typedef enum {
    STATEMENT_RETURN,
    STATEMENT_IF,
} StatementType;
typedef enum {
    EXP_CONSTANT,
    EXP_UNARY_COMPLEMENT,
    EXP_UNARY_NEG,
} ExpressionType;

typedef struct {
    StringView str;
} AST_Identifier;

typedef struct AST_Expression AST_Expression;
struct AST_Expression {
    ExpressionType type;
    union {
        int constant;
        AST_Expression* unary_exp;
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

