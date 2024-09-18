#include "parser.h"

#include <stdio.h>
#include <stdbool.h>

#include "lexer.h"

static VectorTerm terms;
static size_t term_ptr = 0;
static int error = 0;

void eat_term(TermType t) {
    if(terms.array[term_ptr].type == t)
        return;
    printf("expected %s, found %s", 
            TermNames[t], 
            TermNames[terms.array[term_ptr].type]);
    error = 1;
}
#define eat_term_err(t) eat_term(t);if(error)return;
#define eat_term_err_ret(t, ret) eat_term(t);if(error)return ret;
#define eat_term_adv(t) eat_term_err(t)term_ptr++;
#define eat_term_adv_ret(t, ret) eat_term_err_ret(t, ret)term_ptr++;

AST_Expression parse_exp();
AST_Expression* alloc_exp() {
    AST_Expression *ret = malloc(sizeof(AST_Expression));
    *ret = parse_exp();
    return ret;
}

AST_Expression parse_exp_unary(TermType tt, ExpressionType et) {
    AST_Expression ret;
    eat_term_adv_ret(tt, ret);
    ret.type = et;
    ret.unary_exp = alloc_exp();
    return ret;
}

AST_Expression parse_exp() {
    AST_Expression ret;
    switch(terms.array[term_ptr].type) {
        case TERM_CONSTANT:
            eat_term_err_ret(TERM_CONSTANT, ret);
            ret.type = EXP_CONSTANT;
            ret.constant = terms.array[term_ptr].constant;
            term_ptr++;
            return ret;
        case TERM_OPEN_PAR:
            eat_term_adv_ret(TERM_OPEN_PAR, ret);
            ret = parse_exp();
            eat_term_adv_ret(TERM_CLOSE_PAR, ret);
            return ret;
        case TERM_COMPLEMENT:
            return parse_exp_unary(TERM_COMPLEMENT, EXP_UNARY_COMPLEMENT);
        case TERM_NEG:
            return parse_exp_unary(TERM_NEG, EXP_UNARY_NEG);
        default:
            error = 1;
            return ret;
    }
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
    term_ptr = 0;
    error = 0;
    AST_Program ret;
    ret.error = 0;
    ret.function = parse_function();
    if(error)
        ret.error = error;
    return ret;
}

void printf_space(int space) {
    for(int i = 0;i < space;i++)
        printf("    ");
}

void pretty_print_expression(AST_Expression exp) {
    switch(exp.type) {
        case EXP_CONSTANT:
            printf("%d", exp.constant);
            break;
        case EXP_UNARY_COMPLEMENT:
            printf("~");
            pretty_print_expression(*exp.unary_exp);
            break;
        case EXP_UNARY_NEG:
            printf("-");
            pretty_print_expression(*exp.unary_exp);
            break;
    }
}

void pretty_print_statement(AST_Statement s, unsigned space) {
    printf_space(space);
    printf("Return(\n");

    printf_space(space+1);
    pretty_print_expression(s.ret);
    printf("\n");

    printf_space(space);
    printf(")\n");
}

void pretty_print_function(AST_Function f, unsigned space) {
    printf_space(space);
    printf("Function(\n");
    printf_space(space+1);
    printf("name=\"%.*s\"\n", f.name.str.len, f.name.str.start);
    printf_space(space+1);
    printf("body=\n");
    pretty_print_statement(f.statement, space+1);
    printf_space(space);
    printf(")\n");
}

void pretty_print_program(AST_Program program) {  
    printf("Program(\n");
    pretty_print_function(program.function, 1);
    printf(")\n");
}
