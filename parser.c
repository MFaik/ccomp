#include "parser.h"
#include <stdio.h>
#include <stdbool.h>

VectorTerm terms;
size_t term_ptr = 0;
int error = 0;

void eat_term(TermType t) {
    if(terms.array[term_ptr].type == t)
        return;
    printf("expected %d, found %d", t, terms.array[term_ptr].type);
    error = 1;
}
#define eat_term_err(t) eat_term(t);if(error)return;
#define eat_term_err_ret(t, ret) eat_term(t);if(error)return ret;
#define eat_term_adv(t) eat_term_err(t)term_ptr++;
#define eat_term_adv_ret(t, ret) eat_term_err_ret(t, ret)term_ptr++;


AST_Expression parse_exp() {
    AST_Expression ret;
    eat_term_err_ret(TERM_CONSTANT, ret);
    ret.constant = terms.array[term_ptr].constant;
    term_ptr++;
    return ret;
}

AST_Statement parse_statement() {
    AST_Statement ret;
    eat_term_adv_ret(TERM_RETURN, ret);
    ret.ret = parse_exp();if(error)return ret;
    eat_term_adv_ret(TERM_SEMICOLON, ret);
    return ret;
}

AST_Function parse_function() {
    AST_Function ret;
    eat_term_adv_ret(TERM_INT, ret);
    eat_term_err_ret(TERM_IDENTIFIER, ret);
    ret.name = (AST_Identifier){terms.array[term_ptr].s};
    term_ptr++;
    eat_term_adv_ret(TERM_OPEN_PAR, ret);
    eat_term_adv_ret(TERM_VOID, ret);
    eat_term_adv_ret(TERM_CLOSE_PAR, ret);
    eat_term_adv_ret(TERM_OPEN_BRACE, ret);
    ret.statement = parse_statement();if(error)return ret;
    eat_term_adv_ret(TERM_CLOSE_BRACE, ret);
    return ret;
}

AST_Program parse_program(VectorTerm _terms) {
    terms = _terms;
    AST_Program ret;
    ret.error = 0;
    ret.function = parse_function();
    if(error)
        ret.error = error;
    return ret;
}

void pretty_print_statement(AST_Statement s, unsigned space) {
    for(int i = 0;i < space;i++)printf("    ");
    printf("Return(\n");
    for(int i = 0;i <= space;i++)printf("    ");
    printf("%d\n", s.ret.constant);
    for(int i = 0;i < space;i++)printf("    ");
    printf(")\n");
}

void pretty_print_function(AST_Function f, unsigned space) {
    for(int i = 0;i < space;i++)printf("    ");
    printf("Function(\n");
    for(int i = 0;i <= space;i++)printf("    ");
    printf("name=\"%.*s\"\n", f.name.str.len, f.name.str.start);
    for(int i = 0;i <= space;i++)printf("    ");
    printf("body=\n");
    pretty_print_statement(f.statement, space+1);
    for(int i = 0;i < space;i++)printf("    ");
    printf(")\n");
}

void pretty_print_program(AST_Program program) {  
    printf("Program(\n");
    pretty_print_function(program.function, 1);
    printf(")\n");
}
