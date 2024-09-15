#include "lexer.h"
vector_body(Term);

#include <ctype.h>
#include <string.h>

char* code_ptr;

struct {
    TermType tt;
    const char* str;
    int size;
} keyword_str[] = {
    {TERM_INT, "int", 3},
    {TERM_VOID, "void", 4},
    {TERM_RETURN, "return", 6}
};

struct {
    TermType tt;
    const char* str;
    int size;
} symbol_str[] = {
    {TERM_OPEN_PAR, "(", 1},
    {TERM_CLOSE_PAR, ")", 1},
    {TERM_OPEN_BRACE, "{", 1},
    {TERM_CLOSE_BRACE, "}", 1},
    {TERM_SEMICOLON, ";", 1}
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

char is_symbol(char c) {
    return !isalnum(c) && !isspace(c) && c != '_';
}
void eat_symbol(VectorTerm *v) {
    unsigned cnt = 0;
    while(is_symbol(*(code_ptr+cnt)))cnt++;
    Term t;
    for(int i = 0;i < sizeof(symbol_str)/sizeof(symbol_str[0]);i++) {
        if(cnt == symbol_str[i].size &&
                !strncmp(symbol_str[i].str, code_ptr, cnt)) {
            t.type = symbol_str[i].tt;
        }
    }
    insert_vectorTerm(v, t);
    code_ptr += cnt;
}

char make_term(VectorTerm *v) {
    while(isspace(*code_ptr))code_ptr++;
    if(*code_ptr == 0) return 0;
    if(isalpha(*code_ptr)) {
        eat_identifier(v);
        return 1;
    }
    if(isdigit(*code_ptr)) {
        eat_constant(v);
        return 1;
    }
    if(is_symbol(*code_ptr)) {
        eat_symbol(v);
        return 1;
    }
    return 0;
}

VectorTerm lex(char* file) {
    VectorTerm ret;
    init_vectorTerm(&ret, 16);
    code_ptr = file;
    while(make_term(&ret));
    return ret;
}
