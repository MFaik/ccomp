#pragma once

#include "string_view.h"

typedef enum {
    TERM_IDENTIFIER,
    TERM_CONSTANT,
    TERM_OPEN_PAR,
    TERM_CLOSE_PAR,
    TERM_OPEN_BRACE,
    TERM_CLOSE_BRACE,
    TERM_SEMICOLON,
    TERM_INT,
    TERM_VOID,
    TERM_RETURN,
    TERM_GOTO,
    TERM_IF,
    TERM_ELSE,
    TERM_DO,
    TERM_WHILE,
    TERM_FOR,
    TERM_BREAK,
    TERM_CONTINUE,
    TERM_QUESTION,
    TERM_COLON,
    TERM_COMPLEMENT,
    TERM_DECREMENT,
    TERM_INCREMENT,
    TERM_PLUS,
    TERM_MINUS,
    TERM_MUL,
    TERM_DIV,
    TERM_REMAINDER,
    TERM_BITWISE_AND,
    TERM_BITWISE_OR,
    TERM_BITWISE_XOR,
    TERM_LEFT_SHIFT,
    TERM_RIGHT_SHIFT,
    TERM_LOGICAL_NOT,
    TERM_LOGICAL_AND,
    TERM_LOGICAL_OR,
    TERM_EQUAL_TO,
    TERM_NOT_EQUAL,
    TERM_LESS_THAN,
    TERM_GREATER_THAN,
    TERM_LESS_OR_EQUAL,
    TERM_GREATER_OR_EQUAL,
    TERM_ASSIGN,
    TERM_ADD_ASSIGN,
    TERM_SUB_ASSIGN,
    TERM_MUL_ASSIGN,
    TERM_DIV_ASSIGN,
    TERM_REMAINDER_ASSIGN,
    TERM_AND_ASSIGN,
    TERM_OR_ASSIGN,
    TERM_XOR_ASSIGN,
    TERM_LEFT_SHIFT_ASSIGN,
    TERM_RIGHT_SHIFT_ASSIGN,
    TERM_EOF,
} TermType;

extern const char* TermNames[];

typedef struct {
    TermType type;
    union {
        StringView s;
        long constant;
    };
} Term;

#include "vector.h"
vector_header(Term)

VectorTerm lex(char* file);
void pretty_print_term(Term term);
