#include "parser.h"
vector_body(AST_BlockItem);

#include <stdio.h>

#include "lexer.h"

static VectorTerm terms;
static size_t term_ptr = 0;
static int error = 0;

void eat_term(TermType t) {
    if(terms.array[term_ptr].type == t)
        return;
    printf("expected %s, found %s at %lu", 
            TermNames[t], 
            TermNames[terms.array[term_ptr].type], term_ptr);
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

AST_Expression str_to_exp(StringView sv) {
    AST_Expression ret;
    ret.type = EXP_VAR;
    ret.var_str = sv;
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
            break;
        case TERM_MINUS:
            ret = parse_unary_factor(EXP_UNARY_NEG);
            break;
        case TERM_COMPLEMENT:
            ret = parse_unary_factor(EXP_UNARY_COMPLEMENT);
            break;
        case TERM_LOGICAL_NOT:
            ret = parse_unary_factor(EXP_UNARY_LOGICAL_NOT);
            break;
        case TERM_OPEN_PAR:
            term_ptr++;
            ret = parse_exp(0);if(error)return ret;
            eat_term_adv_ret(TERM_CLOSE_PAR, ret);
            break;
        case TERM_IDENTIFIER:
            ret.type = EXP_VAR;
            ret.var_str = terms.array[term_ptr].s;
            term_ptr++;
            break;
        case TERM_INCREMENT:
            ret = parse_unary_factor(EXP_UNARY_PRE_INCREMENT);
            break;
        case TERM_DECREMENT:
            ret = parse_unary_factor(EXP_UNARY_PRE_DECREMENT);
            break;
        default:
            printf("expected factor found %s at %lu", TermNames[terms.array[term_ptr].type], term_ptr);
            error = 2;
            return ret;
    }
    switch(terms.array[term_ptr].type) {
        case TERM_INCREMENT:
            term_ptr++;
            ret.unary_exp = alloc_exp(ret);
            ret.type = EXP_UNARY_POST_INCREMENT;
            break;
        case TERM_DECREMENT:
            term_ptr++;
            ret.unary_exp = alloc_exp(ret);
            ret.type = EXP_UNARY_POST_DECREMENT;
            break;
        default:
            break;
    }
    return ret;
}

unsigned binary_precedence(TermType t) {
    switch(t) {
        case TERM_ASSIGN:
        case TERM_ADD_ASSIGN:
        case TERM_SUB_ASSIGN:
        case TERM_MUL_ASSIGN:
        case TERM_DIV_ASSIGN:
        case TERM_REMAINDER_ASSIGN:
        case TERM_AND_ASSIGN:
        case TERM_OR_ASSIGN:
        case TERM_XOR_ASSIGN:
        case TERM_LEFT_SHIFT_ASSIGN:
        case TERM_RIGHT_SHIFT_ASSIGN:
            return 2;
        case TERM_LOGICAL_OR:
            return 4;
        case TERM_LOGICAL_AND:
            return 5;
        case TERM_BITWISE_OR:
            return 6;
        case TERM_BITWISE_XOR:
            return 7;
        case TERM_BITWISE_AND:
            return 8;
        case TERM_EQUAL_TO:
        case TERM_NOT_EQUAL:
            return 9;
        case TERM_LESS_THAN:
        case TERM_GREATER_THAN:
        case TERM_LESS_OR_EQUAL:
        case TERM_GREATER_OR_EQUAL:
            return 10;
        case TERM_LEFT_SHIFT:
        case TERM_RIGHT_SHIFT:
            return 11;
        case TERM_PLUS:
        case TERM_MINUS:
            return 12;
        case TERM_MUL:
        case TERM_REMAINDER:
        case TERM_DIV:
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
        case TERM_MUL:
            return EXP_BINARY_MUL;
        case TERM_DIV:
            return EXP_BINARY_DIV;
        case TERM_REMAINDER:
            return EXP_BINARY_REMAINDER;
        case TERM_BITWISE_AND:
            return EXP_BINARY_BITWISE_AND;
        case TERM_BITWISE_OR:
            return EXP_BINARY_BITWISE_OR;
        case TERM_BITWISE_XOR:
            return EXP_BINARY_BITWISE_XOR;
        case TERM_LEFT_SHIFT:
            return EXP_BINARY_LEFT_SHIFT;
        case TERM_RIGHT_SHIFT:
            return EXP_BINARY_RIGHT_SHIFT;
        case TERM_LOGICAL_AND:
            return EXP_BINARY_LOGICAL_AND;
        case TERM_LOGICAL_OR:
            return EXP_BINARY_LOGICAL_OR;
        case TERM_EQUAL_TO:
            return EXP_BINARY_EQUAL;
        case TERM_NOT_EQUAL:
            return EXP_BINARY_NOT_EQUAL;
        case TERM_LESS_THAN:
            return EXP_BINARY_LESS_THAN;
        case TERM_GREATER_THAN:
            return EXP_BINARY_GREATER_THAN;
        case TERM_LESS_OR_EQUAL:
            return EXP_BINARY_GREATER_OR_EQUAL;
        case TERM_GREATER_OR_EQUAL:
            return EXP_BINARY_GREATER_OR_EQUAL;
        case TERM_ASSIGN:
            return EXP_BINARY_ASSIGN;
        case TERM_ADD_ASSIGN:
            return EXP_BINARY_ADD_ASSIGN;
        case TERM_SUB_ASSIGN:
            return EXP_BINARY_SUB_ASSIGN;
        case TERM_MUL_ASSIGN:
            return EXP_BINARY_MUL_ASSIGN;
        case TERM_DIV_ASSIGN:
            return EXP_BINARY_DIV_ASSIGN;
        case TERM_REMAINDER_ASSIGN:
            return EXP_BINARY_REMAINDER_ASSIGN;
        case TERM_AND_ASSIGN:
            return EXP_BINARY_AND_ASSIGN;
        case TERM_OR_ASSIGN:
            return EXP_BINARY_OR_ASSIGN;
        case TERM_XOR_ASSIGN:
            return EXP_BINARY_XOR_ASSIGN;
        case TERM_LEFT_SHIFT_ASSIGN:
            return EXP_BINARY_LEFT_SHIFT_ASSIGN;
        case TERM_RIGHT_SHIFT_ASSIGN:
            return EXP_BINARY_RIGHT_SHIFT_ASSIGN;
        default:
            return -1;
    }
}

bool right_assoc(TermType t) {
    return t == TERM_ASSIGN;
}

AST_Expression parse_exp(unsigned min_prec) {
    AST_Expression ret = parse_factor();if(error)return ret;
    unsigned next_prec = binary_precedence(terms.array[term_ptr].type);
    while(term_ptr < terms.size && next_prec > min_prec) {
        ret.left_exp = alloc_exp(ret);
        ret.type = binary_term_to_exp(terms.array[term_ptr].type);
        if(right_assoc(terms.array[term_ptr].type))
            next_prec--;
        term_ptr++;
        ret.right_exp = alloc_exp(parse_exp(next_prec));if(error)return ret;
        next_prec = binary_precedence(terms.array[term_ptr].type);
    }
    return ret;
}

AST_BlockItem parse_statement() {
    if(terms.array[term_ptr].type == TERM_RETURN) {
        AST_BlockItem ret;
        ret.type = AST_STATEMENT_RETURN;
        eat_term_adv_ret(TERM_RETURN, ret);
        ret.exp = parse_exp(0);if(error)return ret;
        eat_term_adv_ret(TERM_SEMICOLON, ret);
        return ret;
    } else if(terms.array[term_ptr].type == TERM_SEMICOLON) {
        term_ptr++;
        return (AST_BlockItem){AST_STATEMENT_NULL};
    } else {
        AST_BlockItem exp;
        exp.type = AST_STATEMENT_EXP;
        exp.exp = parse_exp(0);if(error)return exp;
        eat_term_adv_ret(TERM_SEMICOLON, exp);
        return exp;
    }
}

AST_BlockItem parse_declaration() {
    AST_BlockItem ret;
    ret.type = AST_DECLARATION_NO_ASSIGN;
    eat_term_adv_ret(TERM_INT, ret);
    ret.var = str_to_exp(terms.array[term_ptr].s);
    eat_term_adv_ret(TERM_IDENTIFIER, ret);
    if(terms.array[term_ptr].type == TERM_ASSIGN) {
        ret.type = AST_DECLARATION_WITH_ASSIGN;
        term_ptr++;
        ret.assign_exp = parse_exp(0);if(error)return ret;
    }
    eat_term_adv_ret(TERM_SEMICOLON, ret);
    return ret;
}

AST_BlockItem parse_block_item() {
    if(terms.array[term_ptr].type == TERM_INT) {
        return parse_declaration();
    } else {
        return parse_statement();
    }
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
    //this depends on the fact that a blockitem can't start with a '}'
    init_vectorAST_BlockItem(&ret.block_items, 2);
    while(terms.array[term_ptr].type != TERM_CLOSE_BRACE) {
        AST_BlockItem bi = parse_block_item();if(error)return ret;
        insert_vectorAST_BlockItem(&ret.block_items, bi);
    }
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

static bool resolved;

void printf_space(int space) {
    for(int i = 0;i < space;i++)
        printf("    ");
}

void pretty_print_expression(AST_Expression exp);
void pretty_print_prefix_expression(AST_Expression exp, const char *op) {
    printf("(");
    printf("%s",op);
    pretty_print_expression(*exp.unary_exp);
    printf(")");
}

void pretty_print_postfix_expression(AST_Expression exp, const char *op) {
    printf("(");
    pretty_print_expression(*exp.unary_exp);
    printf("%s",op);
    printf(")");
}

void pretty_print_binary_expression(AST_Expression exp, const char *op) {
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
        case EXP_VAR:
            if(!resolved)
                printf("%.*s", exp.var_str.len, exp.var_str.start);
            else
                printf("$%u", exp.var_id);
            break;
        case EXP_UNARY_COMPLEMENT:
            pretty_print_prefix_expression(exp, "~");
            break;
        case EXP_UNARY_NEG:
            pretty_print_prefix_expression(exp, "-");
            break;
        case EXP_UNARY_LOGICAL_NOT:
            pretty_print_prefix_expression(exp, "!");
            break;
        case EXP_UNARY_PRE_INCREMENT:
            pretty_print_prefix_expression(exp, "++");
            break;
        case EXP_UNARY_PRE_DECREMENT:
            pretty_print_prefix_expression(exp, "--");
            break;
        case EXP_UNARY_POST_INCREMENT:
            pretty_print_postfix_expression(exp, "++");
            break;
        case EXP_UNARY_POST_DECREMENT:
            pretty_print_postfix_expression(exp, "--");
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
        case EXP_BINARY_LOGICAL_AND:
            pretty_print_binary_expression(exp, "&&");
            break;
        case EXP_BINARY_LOGICAL_OR:
            pretty_print_binary_expression(exp, "||");
            break;
        case EXP_BINARY_EQUAL:
            pretty_print_binary_expression(exp, "==");
            break;
        case EXP_BINARY_NOT_EQUAL:
            pretty_print_binary_expression(exp, "!=");
            break;
        case EXP_BINARY_LESS_THAN:
            pretty_print_binary_expression(exp, "<");
            break;
        case EXP_BINARY_GREATER_THAN:
            pretty_print_binary_expression(exp, ">");
            break;
        case EXP_BINARY_LESS_OR_EQUAL:
            pretty_print_binary_expression(exp, "<=");
            break;
        case EXP_BINARY_GREATER_OR_EQUAL:
            pretty_print_binary_expression(exp, ">=");
            break;
        case EXP_BINARY_ASSIGN:
            pretty_print_binary_expression(exp, "=");
            break;
        case EXP_BINARY_ADD_ASSIGN:
            pretty_print_binary_expression(exp, "+=");
            break;
        case EXP_BINARY_SUB_ASSIGN:
            pretty_print_binary_expression(exp, "-=");
            break;
        case EXP_BINARY_MUL_ASSIGN:
            pretty_print_binary_expression(exp, "*=");
            break;
        case EXP_BINARY_DIV_ASSIGN:
            pretty_print_binary_expression(exp, "/=");
            break;
        case EXP_BINARY_REMAINDER_ASSIGN:
            pretty_print_binary_expression(exp, "%=");
            break;
        case EXP_BINARY_AND_ASSIGN:
            pretty_print_binary_expression(exp, "&=");
            break;
        case EXP_BINARY_OR_ASSIGN:
            pretty_print_binary_expression(exp, "|=");
            break;
        case EXP_BINARY_XOR_ASSIGN:
            pretty_print_binary_expression(exp, "^=");
            break;
        case EXP_BINARY_LEFT_SHIFT_ASSIGN:
            pretty_print_binary_expression(exp, "<<=");
            break;
        case EXP_BINARY_RIGHT_SHIFT_ASSIGN:
            pretty_print_binary_expression(exp, ">>=");
            break;
    }
}

void pretty_print_block_item(AST_BlockItem bi, unsigned space) {
    switch(bi.type) {
        case AST_STATEMENT_RETURN:
            printf_space(space);
            printf("return ");
            pretty_print_expression(bi.exp);
            printf(";\n");
            break;
        case AST_STATEMENT_EXP:
            printf_space(space);
            pretty_print_expression(bi.exp);
            printf(";\n");
            break;
        case AST_STATEMENT_NULL:
            printf(";\n");
            break;
        case AST_DECLARATION_NO_ASSIGN:
            printf_space(space);
            printf("int ");
            pretty_print_expression(bi.var);
            printf(";\n");
            break;
        case AST_DECLARATION_WITH_ASSIGN:
            printf_space(space);
            printf("int ");
            pretty_print_expression(bi.var);
            printf(" = ");
            pretty_print_expression(bi.assign_exp);
            printf(";\n");
            break;
    }
}

void pretty_print_function(AST_Function f, unsigned space) {
    printf_space(space);
    printf("Function(\n");
    printf_space(space+1);
    printf("name=\"%.*s\"\n", f.name.str.len, f.name.str.start);
    printf_space(space+1);
    printf("body=\n");

    for(int i = 0;i < f.block_items.size;i++)
        pretty_print_block_item(f.block_items.array[i], space+2);
    printf_space(space);
    printf(")\n");
}

void pretty_print_program(AST_Program program, bool variables_resolved) {  
    resolved = variables_resolved;
    printf("Program(\n");
    pretty_print_function(program.function, 1);
    printf(")\n");
}
