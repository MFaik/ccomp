#pragma once

#include "lexer.h"
#include "string_view.h"

//TODO: haven't used these
typedef enum {
    STATEMENT_RETURN,
    STATEMENT_IF,
} AstType;

typedef struct {
    StringView str;
} AST_Identifier;

typedef struct {
    int constant;
} AST_Expression;

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

