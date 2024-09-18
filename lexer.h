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
    TERM_NEG,
    TERM_COMPLEMENT,
    TERM_DECREMENT,
    TERM_INCREMENT,
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
