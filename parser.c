#include "parser.h"
vector_body(AST_BlockItem);
vector_body(IntLabelPair);

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

AST_BlockItem* alloc_statement(AST_BlockItem statement) {
    AST_BlockItem *ret = malloc(sizeof(AST_BlockItem));
    *ret = statement;
    return ret;
}

AST_BlockItem* alloc_declaration(AST_BlockItem declaration) {
    return alloc_statement(declaration);
}

AST_Expression str_to_exp(StringView sv) {
    AST_Expression ret;
    ret.type = EXP_VAR_STR;
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
            ret.type = EXP_VAR_STR;
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
        case TERM_QUESTION:
            return 3;
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
        case TERM_QUESTION:
            return EXP_CONDITIONAL;
        default:
            return -1;
    }
}

bool is_right_assoc(TermType t) {
    return t == TERM_ASSIGN ||
           t == TERM_QUESTION;
}

AST_Expression parse_exp(unsigned min_prec) {
    AST_Expression ret = parse_factor();if(error)return ret;
    unsigned next_prec = binary_precedence(terms.array[term_ptr].type);
    while(term_ptr < terms.size && next_prec > min_prec) {
        ret.left_exp = alloc_exp(ret);
        ret.type = binary_term_to_exp(terms.array[term_ptr].type);
        if(is_right_assoc(terms.array[term_ptr].type))
            next_prec--;
        term_ptr++;
        if(ret.type == EXP_CONDITIONAL) {
            ret.cond = ret.left_exp;
            ret.true_exp = alloc_exp(parse_exp(0));if(error)return ret;
            eat_term_adv_ret(TERM_COLON, ret);
            ret.false_exp = alloc_exp(parse_exp(next_prec));if(error)return ret;
        } else {
            ret.right_exp = alloc_exp(parse_exp(next_prec));if(error)return ret;
        }
        next_prec = binary_precedence(terms.array[term_ptr].type);
    }
    return ret;
}

bool is_declaration() {
    if(terms.array[term_ptr].type == TERM_INT) {
        return true;
    }
    return false;
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

AST_Block parse_block();
AST_BlockItem parse_statement() {
    if(terms.array[term_ptr].type == TERM_RETURN) {
        term_ptr++;
        AST_BlockItem ret;
        ret.type = AST_STATEMENT_RETURN;
        ret.exp = parse_exp(0);if(error)return ret;
        eat_term_adv_ret(TERM_SEMICOLON, ret);
        return ret;
    } else if(terms.array[term_ptr].type == TERM_SEMICOLON) {
        term_ptr++;
        return (AST_BlockItem){AST_STATEMENT_NULL};
    } else if(terms.array[term_ptr].type == TERM_IF) {
        term_ptr++;
        AST_BlockItem ret;
        ret.type = AST_STATEMENT_IF;
        eat_term_adv_ret(TERM_OPEN_PAR, ret);
        ret.if_cond = parse_exp(0);if(error)return ret;
        eat_term_adv_ret(TERM_CLOSE_PAR, ret);
        ret.then = alloc_statement(parse_statement());
        if(terms.array[term_ptr].type == TERM_ELSE) {
            term_ptr++;
            ret.type = AST_STATEMENT_IF_ELSE;
            ret.else_ = alloc_statement(parse_statement());
        }
        return ret;
    } else if(terms.array[term_ptr].type == TERM_GOTO) {
        term_ptr++;
        AST_BlockItem ret;
        ret.type = AST_STATEMENT_GOTO;
        ret.exp = str_to_exp(terms.array[term_ptr].s);
        eat_term_adv_ret(TERM_IDENTIFIER, ret);
        return ret;
    } else if(terms.array[term_ptr].type == TERM_DO) {
        term_ptr++;
        AST_BlockItem ret;
        ret.type = AST_STATEMENT_DO_WHILE;
        ret.loop_body = alloc_statement(parse_statement());
        eat_term_adv_ret(TERM_WHILE, ret);
        eat_term_adv_ret(TERM_OPEN_PAR, ret);
        ret.loop_cond = parse_exp(0);
        eat_term_adv_ret(TERM_CLOSE_PAR, ret);
        eat_term_adv_ret(TERM_SEMICOLON, ret);
        return ret;
    } else if(terms.array[term_ptr].type == TERM_WHILE) {
        term_ptr++;
        AST_BlockItem ret;
        ret.type = AST_STATEMENT_WHILE;
        eat_term_adv_ret(TERM_OPEN_PAR, ret);
        ret.loop_cond = parse_exp(0);
        eat_term_adv_ret(TERM_CLOSE_PAR, ret);
        ret.loop_body = alloc_statement(parse_statement());
        return ret;
    } else if(terms.array[term_ptr].type == TERM_FOR) {
        term_ptr++;
        AST_BlockItem ret;
        ret.type = AST_STATEMENT_FOR;
        AST_Expression exp_1;
        exp_1.type = EXP_CONSTANT;
        exp_1.constant = 1;
        ret.loop_cond = exp_1;
        ret.loop_it = exp_1;
        eat_term_adv_ret(TERM_OPEN_PAR, ret);
        if(is_declaration()) {
            ret.init = alloc_declaration(parse_declaration());
        } else {
            AST_BlockItem init;
            init.type = AST_STATEMENT_EXP;
            if(terms.array[term_ptr].type == TERM_SEMICOLON) {
                init.exp = exp_1;
            } else {
                init.exp = parse_exp(0);
            }
            ret.init = alloc_statement(init);
            eat_term_adv_ret(TERM_SEMICOLON, ret);
        }
        if(terms.array[term_ptr].type != TERM_SEMICOLON)
            ret.loop_cond = parse_exp(0);
        eat_term_adv_ret(TERM_SEMICOLON, ret);
        if(terms.array[term_ptr].type != TERM_CLOSE_PAR)
            ret.loop_it = parse_exp(0);
        eat_term_adv_ret(TERM_CLOSE_PAR, ret);
        ret.loop_body = alloc_statement(parse_statement());
        return ret;
    } else if(terms.array[term_ptr].type == TERM_CONTINUE) {
        term_ptr++;
        AST_BlockItem ret;
        ret.type = AST_STATEMENT_CONTINUE;
        eat_term_adv_ret(TERM_SEMICOLON, ret);
        return ret;
    } else if(terms.array[term_ptr].type == TERM_BREAK) {
        term_ptr++;
        AST_BlockItem ret;
        ret.type = AST_STATEMENT_BREAK;
        eat_term_adv_ret(TERM_SEMICOLON, ret);
        return ret;
    } else if(terms.array[term_ptr].type == TERM_SWITCH) {
        term_ptr++;
        AST_BlockItem ret;
        ret.type = AST_STATEMENT_SWITCH;
        eat_term_adv_ret(TERM_OPEN_PAR, ret);
        ret.switch_exp = parse_exp(0);
        eat_term_adv_ret(TERM_CLOSE_PAR, ret);
        ret.switch_body = alloc_statement(parse_statement());
        return ret;
    } else if(terms.array[term_ptr].type == TERM_OPEN_BRACE){
        AST_BlockItem ret;
        ret.type = AST_STATEMENT_COMPOUND;
        ret.block = parse_block();
        return ret;
    } else {
        AST_BlockItem exp;
        exp.type = AST_STATEMENT_EXP;
        exp.exp = parse_exp(0);if(error)return exp;
        eat_term_adv_ret(TERM_SEMICOLON, exp);
        return exp;
    }
}

bool is_label() {
    if(terms.array[term_ptr].type == TERM_CASE)
        return true;
    if(terms.array[term_ptr].type == TERM_IDENTIFIER &&
       terms.array[term_ptr+1].type == TERM_COLON)
        return true;
    if(terms.array[term_ptr].type == TERM_DEFAULT && 
       terms.array[term_ptr+1].type == TERM_COLON)
        return true;
    return false;
}
AST_BlockItem parse_label() {
    AST_BlockItem ret;
    if(terms.array[term_ptr].type == TERM_IDENTIFIER) {
        ret.type = AST_LABEL;
        ret.exp = str_to_exp(terms.array[term_ptr].s); 
        term_ptr += 2;
        return ret;
    }
    if(terms.array[term_ptr].type == TERM_DEFAULT) {
        ret.type = AST_DEFAULT_LABEL;
        term_ptr += 2;
        return ret;
    }
    //case label
    ret.type = AST_CASE_LABEL;
    term_ptr++;//eat_term_adv_ret(TERM_CASE, ret);
    ret.exp = parse_exp(0);
    eat_term_adv_ret(TERM_COLON, ret);
    return ret;
}

AST_BlockItem parse_block_item() {
    if(is_label()) {
        return parse_label();
    } else if(is_declaration()) {
        return parse_declaration();
    } else {
        return parse_statement();
    }
}

AST_Block parse_block() {
    AST_Block ret;
    init_vectorAST_BlockItem(&ret.block_items, 2);
    eat_term_adv_ret(TERM_OPEN_BRACE, ret);
    while(terms.array[term_ptr].type != TERM_CLOSE_BRACE) {
        push_vectorAST_BlockItem(&ret.block_items, parse_block_item());
        if(error)return ret;
    }
    term_ptr++;//eat_term_adv_ret(TERM_CLOSE_BRACE, ret);
    return ret;
}

AST_Function parse_function() {
    AST_Function ret;
    eat_term_adv_ret(TERM_INT, ret);
    eat_term_err_ret(TERM_IDENTIFIER, ret);//doesn't advance
    ret.name = (AST_Identifier){terms.array[term_ptr].s};
    term_ptr++;
    eat_term_adv_ret(TERM_OPEN_PAR, ret);
    eat_term_adv_ret(TERM_VOID, ret);
    eat_term_adv_ret(TERM_CLOSE_PAR, ret);
    ret.block = parse_block();
    return ret;
}

AST_Program parse_program(VectorTerm _terms) {
    terms = _terms;
    term_ptr = 0;
    error = 0;
    AST_Program ret;
    ret.error = 0;
    ret.function = parse_function();
    if(!error)
        eat_term(TERM_EOF);

    if(error)
        ret.error = error;
    return ret;
}

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
        case EXP_VAR_STR:
            printf("%.*s", exp.var_str.len, exp.var_str.start);
            break;
        case EXP_VAR_ID:
            printf("$%u", exp.var_id);
            break;
        case EXP_CONDITIONAL:
            printf("(");
            pretty_print_expression(*exp.cond);
            printf(" ? ");
            pretty_print_expression(*exp.true_exp);
            printf(" : ");
            pretty_print_expression(*exp.false_exp);
            printf(")");
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
    if(bi.type == AST_STATEMENT_COMPOUND)
        space--;
    printf_space(space);
    switch(bi.type) {
        case AST_STATEMENT_RETURN:
            printf("return ");
            pretty_print_expression(bi.exp);
            printf(";");
            break;
        case AST_STATEMENT_EXP:
            pretty_print_expression(bi.exp);
            printf(";");
            break;
        case AST_STATEMENT_NULL:
            printf(";");
            break;
        case AST_STATEMENT_IF:
            printf("if(");
            pretty_print_expression(bi.if_cond);
            printf(")\n");
            pretty_print_block_item(*bi.then, space+1);
            break;
        case AST_STATEMENT_IF_ELSE:
            printf("if( ");
            pretty_print_expression(bi.if_cond);
            printf(" )\n");
            pretty_print_block_item(*bi.then, space+1);
            printf_space(space);
            printf("else\n");
            pretty_print_block_item(*bi.else_, space+1);
            break;
        case AST_STATEMENT_FOR:
            printf("for(");
            pretty_print_block_item(*bi.init, 0);
            pretty_print_expression(bi.loop_cond);
            printf(";");
            pretty_print_expression(bi.loop_it);
            printf(")%u\n", bi.loop_id);
            pretty_print_block_item(*bi.loop_body, space+1);
            break;
        case AST_STATEMENT_DO_WHILE:
            printf("do\n");
            pretty_print_block_item(*bi.loop_body, space+1);
            printf_space(space);
            printf("while(");
            pretty_print_expression(bi.loop_cond);
            printf(");%u", bi.loop_id);
            break;
        case AST_STATEMENT_WHILE:
            printf("while(");
            pretty_print_expression(bi.loop_cond);
            printf(")%u\n", bi.loop_id);
            pretty_print_block_item(*bi.loop_body, space+1);
            break;
        case AST_STATEMENT_BREAK:
            printf("break;%u", bi.exp.var_id);
            break;
        case AST_STATEMENT_CONTINUE:
            printf("continue;%u", bi.exp.var_id);
            break;
        case AST_STATEMENT_GOTO:
            printf("goto ");
            pretty_print_expression(bi.exp);
            printf(";");
            break;
        case AST_STATEMENT_SWITCH:
            printf("switch(");
            pretty_print_expression(bi.if_cond);
            printf(")%u\n", bi.switch_id);
            pretty_print_block_item(*bi.then, space+1);
            break;
        case AST_LABEL:
            pretty_print_expression(bi.exp);
            printf(":");
            break;
        case AST_DEFAULT_LABEL:
            printf("default:");
            pretty_print_expression(bi.exp);
            break;
        case AST_CASE_LABEL:
            printf("case ");
            pretty_print_expression(bi.exp);
            printf(":");
            break;
        case AST_STATEMENT_COMPOUND:
            printf("{\n");
            for(int i = 0;i < bi.block.block_items.size;i++) {
                pretty_print_block_item(bi.block.block_items.array[i], space+1);
            }
            printf_space(space);
            printf("}");
            break;
        case AST_DECLARATION_NO_ASSIGN:
            printf("int ");
            pretty_print_expression(bi.var);
            printf(";");
            break;
        case AST_DECLARATION_WITH_ASSIGN:
            printf("int ");
            pretty_print_expression(bi.var);
            printf(" = ");
            pretty_print_expression(bi.assign_exp);
            printf(";");
            break;
    }
    if(space > 0)
        printf("\n");
}

void pretty_print_function(AST_Function f, unsigned space) {
    printf_space(space);
    printf("Function(\n");
    printf_space(space+1);
    printf("name=\"%.*s\"\n", f.name.str.len, f.name.str.start);
    printf_space(space+1);
    printf("body=\n");

    for(int i = 0;i < f.block.block_items.size;i++)
        pretty_print_block_item(f.block.block_items.array[i], space+2);
    printf_space(space);
    printf(")\n");
}

void pretty_print_program(AST_Program program) {  
    printf("Program(\n");
    pretty_print_function(program.function, 1);
    printf(")\n");
}
