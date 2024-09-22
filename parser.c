#include "parser.h"

#include <stdio.h>
#include <stdbool.h>

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

AST_Expression* alloc_exp(AST_Expression exp) {
    AST_Expression *ret = malloc(sizeof(AST_Expression));
    *ret = exp;
    return ret;
}

AST_Expression parse_factor();
AST_Expression parse_unary_factor(AST_ExpressionType t) {
    term_ptr++;
    AST_Expression ret;
    ret.type = t;
    ret.unary_exp = alloc_exp(parse_factor());
    return ret;
}

AST_Expression parse_exp(unsigned);
AST_Expression parse_factor() {
    AST_Expression ret;
    switch(terms.array[term_ptr].type) {
        case TERM_CONSTANT:
            ret.type = EXP_CONSTANT;
            ret.constant = terms.array[term_ptr].constant;
            term_ptr++;
            return ret;
        case TERM_MINUS:
            return parse_unary_factor(EXP_UNARY_NEG);
        case TERM_COMPLEMENT:
            return parse_unary_factor(EXP_UNARY_COMPLEMENT);
        case TERM_OPEN_PAR:
            term_ptr++;
            ret = parse_exp(0);if(error)return ret;
            eat_term_adv_ret(TERM_CLOSE_PAR, ret);
            return ret;
        default:
            printf("expected factor found %s", TermNames[terms.array[term_ptr].type]);
            error = 2;
            return ret;
    }
}

unsigned binary_precedence(TermType t) {
    switch(t) {
        case TERM_PIPE:
            return 6;
        case TERM_HAT:
            return 7;
        case TERM_AMPERSAND:
            return 8;
        case TERM_LEFT_SHIFT:
        case TERM_RIGHT_SHIFT:
            return 11;
        case TERM_PLUS:
        case TERM_MINUS:
            return 12;
        case TERM_ASTERISK:
        case TERM_PERCENT:
        case TERM_FWD_SLASH:
            return 13;
        default:
            return 0;
    }
}

AST_ExpressionType binary_term_to_exp(TermType t) {
    switch(t) {
        case TERM_PLUS:
            return EXP_BINARY_ADD;
        case TERM_MINUS:
            return EXP_BINARY_SUB;
        case TERM_ASTERISK:
            return EXP_BINARY_MUL;
        case TERM_FWD_SLASH:
            return EXP_BINARY_DIV;
        case TERM_PERCENT:
            return EXP_BINARY_REMAINDER;
        case TERM_AMPERSAND:
            return EXP_BINARY_BITWISE_AND;
        case TERM_PIPE:
            return EXP_BINARY_BITWISE_OR;
        case TERM_HAT:
            return EXP_BINARY_BITWISE_XOR;
        case TERM_LEFT_SHIFT:
            return EXP_BINARY_LEFT_SHIFT;
        case TERM_RIGHT_SHIFT:
            return EXP_BINARY_RIGHT_SHIFT;
        default:
            return -1;
    }
}

AST_Expression parse_exp(unsigned min_prec) {
    AST_Expression ret = parse_factor();if(error)return ret;
    unsigned next_prec = binary_precedence(terms.array[term_ptr].type);
    while(term_ptr < terms.size && next_prec > min_prec) {
        ret.left_exp = alloc_exp(ret);
        ret.type = binary_term_to_exp(terms.array[term_ptr].type);
        term_ptr++;
        ret.right_exp = alloc_exp(parse_exp(next_prec));if(error)return ret;
        next_prec = binary_precedence(terms.array[term_ptr].type);
    }
    return ret;
}

AST_Statement parse_statement() {
    AST_Statement ret;
    eat_term_adv_ret(TERM_RETURN, ret);
    ret.ret = parse_exp(0);if(error)return ret;
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

void pretty_print_expression(AST_Expression exp);
void pretty_print_unary_expression(AST_Expression exp, char op) {
    printf("(");
    printf("%c",op);
    pretty_print_expression(*exp.unary_exp);
    printf(")");
}

void pretty_print_binary_expression(AST_Expression exp, const char* op) {
    printf("(");
    pretty_print_expression(*exp.left_exp);
    printf("%s",op);
    pretty_print_expression(*exp.right_exp);
    printf(")");
}

void pretty_print_expression(AST_Expression exp) {
    switch(exp.type) {
        case EXP_CONSTANT:
            printf("%d", exp.constant);
            break;
        case EXP_UNARY_COMPLEMENT:
            pretty_print_unary_expression(exp, '~');
            break;
        case EXP_UNARY_NEG:
            pretty_print_unary_expression(exp, '-');
            break;
        case EXP_BINARY_ADD:
            pretty_print_binary_expression(exp, "+");
            break;
        case EXP_BINARY_SUB:
            pretty_print_binary_expression(exp, "-");
            break;
        case EXP_BINARY_MUL:
            pretty_print_binary_expression(exp, "*");
            break;
        case EXP_BINARY_DIV:
            pretty_print_binary_expression(exp, "/");
            break;
        case EXP_BINARY_REMAINDER:
            pretty_print_binary_expression(exp, "%");
            break;
        case EXP_BINARY_BITWISE_AND:
            pretty_print_binary_expression(exp, "&");
            break;
        case EXP_BINARY_BITWISE_OR:
            pretty_print_binary_expression(exp, "|");
            break;
        case EXP_BINARY_BITWISE_XOR:
            pretty_print_binary_expression(exp, "^");
            break;
        case EXP_BINARY_LEFT_SHIFT:
            pretty_print_binary_expression(exp, "<<");
            break;
        case EXP_BINARY_RIGHT_SHIFT:
            pretty_print_binary_expression(exp, ">>");
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
