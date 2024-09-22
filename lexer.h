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
    TERM_COMPLEMENT,
    TERM_DECREMENT,
    TERM_INCREMENT,
    TERM_PLUS,
    TERM_MINUS,
    TERM_ASTERISK,
    TERM_FWD_SLASH,
    TERM_PERCENT,
    TERM_AMPERSAND,
    TERM_PIPE,
    TERM_HAT,
    TERM_LEFT_SHIFT,
    TERM_RIGHT_SHIFT,
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
