#include "semantic_anal.h"

#include <stdio.h>

#include "parser.h"
#include "map.h"
#include "vector.h"
vector_header(SVMap);vector_body(SVMap);

static int error = 0;
static unsigned map_cnt = 0;
static VectorSVMap *var_stack;

unsigned resolve_new_var(AST_Expression *exp) {
    if(get_map(top_vectorSVMap(var_stack), exp->var_str) != ID_NOT_FOUND) {
        error = 1;
        printf("Redefinition of %.*s\n", exp->var_str.len, exp->var_str.start);
        return ID_NOT_FOUND;
    }
    return insert_map(top_vectorSVMap(var_stack), exp->var_str, map_cnt++);
}

unsigned resolve_exist_var(AST_Expression *exp) {
    unsigned id = ID_NOT_FOUND;
    for(int i = var_stack->size-1;i >= 0;i--) {
        id = get_map(&var_stack->array[i], exp->var_str);
        if(id != ID_NOT_FOUND)
            return id;
    }
    error = 2;
    printf("use of undeclared identifier %.*s\n", exp->var_str.len, exp->var_str.start);
    return ID_NOT_FOUND;
}

bool is_exp_var(AST_Expression *exp) {
    if(exp->type == EXP_VAR_STR || exp->type == EXP_VAR_ID) {
        return true;
    }
    return false;
}

void var_resolve_expression(AST_Expression *exp) {
    switch(exp->type) {
        case EXP_VAR_STR:
            exp->type = EXP_VAR_ID;
            exp->var_id = resolve_exist_var(exp);
            break;
        case EXP_VAR_ID:
            break;
        case EXP_CONDITIONAL:
            var_resolve_expression(exp->cond);
            var_resolve_expression(exp->true_exp);
            var_resolve_expression(exp->false_exp);
            break;
        case EXP_CONSTANT:
            break;
        case EXP_UNARY_COMPLEMENT:
        case EXP_UNARY_NEG:
        case EXP_UNARY_LOGICAL_NOT:
            var_resolve_expression(exp->unary_exp);
            break;
        case EXP_UNARY_PRE_INCREMENT:
        case EXP_UNARY_PRE_DECREMENT:
        case EXP_UNARY_POST_INCREMENT:
        case EXP_UNARY_POST_DECREMENT:
            if(!is_exp_var(exp->unary_exp)) {
                printf("expression cannot be assigned to\n");
                error = 3;
                return;
            }
            var_resolve_expression(exp->unary_exp);
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
            var_resolve_expression(exp->left_exp);
            var_resolve_expression(exp->right_exp);
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
            if(!is_exp_var(exp->left_exp)) {
                printf("expression cannot be assigned to\n");
                error = 3;
                return;
            }
            var_resolve_expression(exp->left_exp);
            var_resolve_expression(exp->right_exp);
            break;
    }
}

void var_resolve_declaration(AST_BlockItem *bi) {
    resolve_new_var(&bi->var);
    var_resolve_expression(&bi->var);
    if(bi->type == AST_DECLARATION_WITH_ASSIGN)
        var_resolve_expression(&bi->assign_exp);
}

void var_resolve_block(AST_Block *block);;
void var_resolve_statement(AST_BlockItem *bi) {
    if(bi->type == AST_STATEMENT_IF || bi->type == AST_STATEMENT_IF_ELSE) {
        var_resolve_expression(&bi->cond);
        var_resolve_statement(bi->then);
        if(bi->type == AST_STATEMENT_IF_ELSE)
            var_resolve_statement(bi->else_);
    } else if(bi->type == AST_STATEMENT_COMPOUND) {
        var_resolve_block(&bi->block);
    } else {
        if(bi->type != AST_STATEMENT_NULL)
            var_resolve_expression(&bi->exp);
    }
}

void var_resolve_block(AST_Block *block) {
    SVMap map;
    init_map(&map);
    push_vectorSVMap(var_stack, map);

    for(int i = 0;i < block->block_items.size;i++) {
        switch(block->block_items.array[i].type) {
            case AST_STATEMENT_RETURN:
            case AST_STATEMENT_EXP:
            case AST_STATEMENT_NULL:
            case AST_STATEMENT_IF:
            case AST_STATEMENT_IF_ELSE:
            case AST_STATEMENT_COMPOUND:
                var_resolve_statement(&block->block_items.array[i]);
                break;
            case AST_DECLARATION_NO_ASSIGN:
            case AST_DECLARATION_WITH_ASSIGN:
                var_resolve_declaration(&block->block_items.array[i]);
                break;
            case AST_STATEMENT_GOTO:
            case AST_LABEL:
                break;
        }
    }

    free_map(top_vectorSVMap(var_stack));
    pop_vectorSVMap(var_stack);
}

void var_resolve_function(AST_Function *f) {
    var_resolve_block(&f->block);
}

void var_resolve_program(AST_Program *p) {
    map_cnt = 0;

    var_resolve_function(&p->function);
    p->var_cnt = map_cnt;
}

void label_resolve_block(AST_Block *b, bool is_label);
void label_resolve_block_item(AST_BlockItem *bi, bool is_label, bool last) {
    if(bi->type == AST_STATEMENT_COMPOUND) {
        label_resolve_block(&bi->block, is_label);
    } else if(bi->type == AST_STATEMENT_IF) {
        label_resolve_block_item(bi->then, is_label, true);
    } else if(bi->type == AST_STATEMENT_IF_ELSE) {
        label_resolve_block_item(bi->then, is_label, true);
        label_resolve_block_item(bi->else_, is_label, true);
    } else if(is_label && bi->type == AST_LABEL) {
        bi->exp.type = EXP_VAR_ID;
        bi->exp.var_id = resolve_new_var(&bi->exp);
        if(last) {
            error = 4;
            printf("expected expression after label %.*s\n", 
                    bi->exp.var_str.len, bi->exp.var_str.start);
        }
    } else if(!is_label && bi->type == AST_STATEMENT_GOTO) {
        bi->exp.type = EXP_VAR_ID;
        bi->exp.var_id = resolve_exist_var(&bi->exp);
    }
}

void label_resolve_block(AST_Block *b, bool is_label) {
    for(int i = 0;i < b->block_items.size;i++) {
        AST_BlockItem *bi = &b->block_items.array[i];
        label_resolve_block_item(bi, is_label, i == b->block_items.size-1);
    }
}

void label_resolve_program(AST_Program *p) {
    map_cnt = 0;
    SVMap map;
    init_map(&map);
    push_vectorSVMap(var_stack, map);

    label_resolve_block(&p->function.block, true);
    label_resolve_block(&p->function.block, false);

    p->label_cnt = map_cnt;

    free_map(&map);
    pop_vectorSVMap(var_stack);
}

void resolve_program(AST_Program *p) {
    error = 0;
    VectorSVMap vs;
    init_vectorSVMap(&vs, 2);
    var_stack = &vs;

    var_resolve_program(p);
    p->error = error;
    if(error)return;
    label_resolve_program(p);
    p->error = error;
}
