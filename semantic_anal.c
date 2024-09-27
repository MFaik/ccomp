#include "semantic_anal.h"

#include <stdio.h>

#include "map.h"
#include "parser.h"

static int error = 0;
static unsigned var_cnt = 0;

void resolve_new_var(AST_Expression *exp, SVMap* var_map) {
    if(exp->type != EXP_VAR || get_map(var_map, exp->var_str) != ID_NOT_FOUND) {
        error = 1;
        printf("Redefinition of %.*s\n", exp->var_str.len, exp->var_str.start);
        return;
    }
    exp->var_id = insert_map(var_map, exp->var_str);
}

void resolve_expression(AST_Expression *exp, SVMap* var_map) {
    switch(exp->type) {
        case EXP_VAR:
            exp->var_id = get_map(var_map, exp->var_str);
            if(exp->var_id == ID_NOT_FOUND) {
                error = 2;
                printf("use of undeclared identifier %.*s\n", exp->var_str.len, exp->var_str.start);
                return;
            }
            break;
        case EXP_CONDITIONAL:
            resolve_expression(exp->cond, var_map);
            resolve_expression(exp->true_exp, var_map);
            resolve_expression(exp->false_exp, var_map);
            break;
        case EXP_CONSTANT:
            break;
        case EXP_UNARY_COMPLEMENT:
        case EXP_UNARY_NEG:
        case EXP_UNARY_LOGICAL_NOT:
            resolve_expression(exp->unary_exp, var_map);
            break;
        case EXP_UNARY_PRE_INCREMENT:
        case EXP_UNARY_PRE_DECREMENT:
        case EXP_UNARY_POST_INCREMENT:
        case EXP_UNARY_POST_DECREMENT:
            if(exp->unary_exp->type != EXP_VAR) {
                printf("expression cannot be assigned to\n");
                error = 3;
                return;
            }
            resolve_expression(exp->unary_exp, var_map);
            break;
        case EXP_BINARY_ADD:
        case EXP_BINARY_SUB:
        case EXP_BINARY_MUL:
        case EXP_BINARY_DIV:
        case EXP_BINARY_REMAINDER:
        case EXP_BINARY_LEFT_SHIFT:
        case EXP_BINARY_RIGHT_SHIFT:
        case EXP_BINARY_BITWISE_AND:
        case EXP_BINARY_BITWISE_OR:
        case EXP_BINARY_BITWISE_XOR:
        case EXP_BINARY_LOGICAL_AND:
        case EXP_BINARY_LOGICAL_OR:
        case EXP_BINARY_EQUAL:
        case EXP_BINARY_NOT_EQUAL:
        case EXP_BINARY_LESS_THAN:
        case EXP_BINARY_GREATER_THAN:
        case EXP_BINARY_LESS_OR_EQUAL:
        case EXP_BINARY_GREATER_OR_EQUAL:
            resolve_expression(exp->left_exp, var_map);
            resolve_expression(exp->right_exp, var_map);
            break;
        case EXP_BINARY_ASSIGN:
        case EXP_BINARY_ADD_ASSIGN:
        case EXP_BINARY_SUB_ASSIGN:
        case EXP_BINARY_MUL_ASSIGN:
        case EXP_BINARY_DIV_ASSIGN:
        case EXP_BINARY_REMAINDER_ASSIGN:
        case EXP_BINARY_AND_ASSIGN:
        case EXP_BINARY_OR_ASSIGN:
        case EXP_BINARY_XOR_ASSIGN:
        case EXP_BINARY_LEFT_SHIFT_ASSIGN:
        case EXP_BINARY_RIGHT_SHIFT_ASSIGN:
            if(exp->left_exp->type != EXP_VAR) {
                printf("expression cannot be assigned to\n");
                error = 3;
                return;
            }
            resolve_expression(exp->left_exp, var_map);
            resolve_expression(exp->right_exp, var_map);
            break;
    }
}

void resolve_declaration(AST_BlockItem *bi, SVMap *var_map) {
    resolve_new_var(&bi->var, var_map);
    if(bi->type == AST_DECLARATION_WITH_ASSIGN)
        resolve_expression(&bi->assign_exp, var_map);
}

void resolve_statement(AST_BlockItem *bi, SVMap *var_map) {
    if(bi->type == AST_STATEMENT_IF || bi->type == AST_STATEMENT_IF_ELSE) {
        resolve_expression(&bi->cond, var_map);
        resolve_statement(bi->then, var_map);
        if(bi->type == AST_STATEMENT_IF_ELSE)
            resolve_statement(bi->else_, var_map);
    } else {
        if(bi->type != AST_STATEMENT_NULL)
            resolve_expression(&bi->exp, var_map);
    }
}

void resolve_function(AST_Function *f) {
    SVMap var_map;
    init_map(&var_map, var_cnt);
    for(int i = 0;i < f->block_items.size;i++) {
        switch(f->block_items.array[i].type) {
            case AST_STATEMENT_RETURN:
            case AST_STATEMENT_EXP:
            case AST_STATEMENT_NULL:
            case AST_STATEMENT_IF:
            case AST_STATEMENT_IF_ELSE:
                resolve_statement(&f->block_items.array[i], &var_map);
                break;
            case AST_DECLARATION_NO_ASSIGN:
            case AST_DECLARATION_WITH_ASSIGN:
                resolve_declaration(&f->block_items.array[i], &var_map);
                break;
        }
    }
    var_cnt += var_map.size;
}

void resolve_program(AST_Program *p) {
    error = 0;
    var_cnt = 0;
    resolve_function(&p->function);
    p->error = error;
    p->var_cnt = var_cnt;
}
