#include "lexer.h"
vector_body(Term);
const char* TermNames[] = {
    "identifier",
    "constant",
    "open_par",
    "close_par",
    "open_brace",
    "close_brace",
    "semicolon",
    "int",
    "void",
    "return",
    "complement",
    "decrement",
    "increment",
    "plus",
    "minus",
    "asterisk",
    "forward-slash",
    "percent",
    "ampersand",
    "pipe",
    "hat",
    "left-shift",
    "right-shift",
    "end of file",
};

#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

static char* code_ptr;
static int error = 0;

struct {
    TermType tt;
    const char* str;
    int size;
} keyword_str[] = {
    {TERM_INT, "int", 3},
    {TERM_VOID, "void", 4},
    {TERM_RETURN, "return", 6}
};

//the symbols are in decreasing order 
//as the longer ones take priority
struct {
    TermType tt;
    const char* str;
    int size;
} symbol_str[] = {
    //two char symbols
    {TERM_DECREMENT, "--", 2},
    {TERM_INCREMENT, "++", 2},
    {TERM_LEFT_SHIFT, "<<", 2},
    {TERM_RIGHT_SHIFT, ">>", 2},
    //one char symbols
    {TERM_OPEN_PAR, "(", 1},
    {TERM_CLOSE_PAR, ")", 1},
    {TERM_OPEN_BRACE, "{", 1},
    {TERM_CLOSE_BRACE, "}", 1},
    {TERM_SEMICOLON, ";", 1},
    {TERM_COMPLEMENT, "~", 1},
    {TERM_PLUS, "+", 1},
    {TERM_MINUS, "-", 1},
    {TERM_ASTERISK, "*", 1},
    {TERM_FWD_SLASH, "/", 1},
    {TERM_PERCENT, "%", 1},
    {TERM_AMPERSAND, "&", 1},
    {TERM_PIPE, "|", 1},
    {TERM_HAT, "^", 1},
};

void eat_identifier(VectorTerm *v) {
    unsigned cnt = 0;
    while(isalnum(*(code_ptr+cnt)) || *(code_ptr+cnt) == '_')cnt++;
    Term t;
    t.type = -1;
    for(int i = 0;i < sizeof(keyword_str)/sizeof(keyword_str[0]);i++) {
        if(cnt == keyword_str[i].size && 
                !strncmp(keyword_str[i].str, code_ptr, cnt)) {
            t.type = keyword_str[i].tt;
            break;
        }
    }
    if(t.type == -1) {
        t.type = TERM_IDENTIFIER;
        t.s = (StringView){code_ptr, cnt};
    }
    insert_vectorTerm(v, t);
    code_ptr += cnt;
}

void eat_constant(VectorTerm *v) {
    unsigned cnt = 0;
    long constant = 0;
    while(isdigit(*(code_ptr+cnt))) {
        constant *= 10;
        constant += *code_ptr-'0';
        cnt++;
    }
    Term t;
    t.type = TERM_CONSTANT;
    t.constant = constant;
    insert_vectorTerm(v, t);
    code_ptr += cnt;
}

bool is_paran(char c) {
    return c=='('||c==')'||c=='['||c==']'||c=='{'||c=='}';
}
bool is_symbol(char c) {
    return !isalnum(c) && !isspace(c) && c != '_';
}
bool eat_symbol(VectorTerm *v) {
    unsigned cnt = 0;
    Term t;
    t.type = -1;
    for(int i = 0;i < sizeof(symbol_str)/sizeof(symbol_str[0]);i++) {
        if(!strncmp(symbol_str[i].str, code_ptr, symbol_str[i].size)) {
            t.type = symbol_str[i].tt;
            cnt = symbol_str[i].size;
            break;
        }
    }
    if(t.type == -1) {
        return false;
    }
    insert_vectorTerm(v, t);
    code_ptr += cnt;
    return true;
}

bool make_term(VectorTerm *v) {
    while(isspace(*code_ptr))code_ptr++;
    if(*code_ptr == 0) return false;
    if(isalpha(*code_ptr)) {
        eat_identifier(v);
        return true;
    }
    if(isdigit(*code_ptr)) {
        eat_constant(v);
        return true;
    }
    if(is_symbol(*code_ptr)) {
        if(!eat_symbol(v)) {
            printf("unknown symbol");
            error = 1;
            return false;
        }
        return true;
    }
    return false;
}

VectorTerm lex(char* file) {
    error = 0;
    VectorTerm ret;
    init_vectorTerm(&ret, 16);
    code_ptr = file;
    while(make_term(&ret));
    if(error) {
        ret.size = 0;
    }
    insert_vectorTerm(&ret, (Term){TERM_EOF});
    return ret;
}

void pretty_print_term(Term term) {
    printf("%s", TermNames[term.type]);
    if(term.type == TERM_IDENTIFIER) {
        printf(" %.*s", term.s.len, term.s.start);
    } else if(term.type == TERM_CONSTANT) {
        printf(" %ld", term.constant);
    }
}
