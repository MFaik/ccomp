#include "semantic_anal.h"

#include <stdio.h>

#include "parser.h"
#include "map.h"
#include "vector.h"
vector_header(SVMap);vector_body(SVMap);

static int error = 0;
static unsigned map_cnt = 0;
static VectorSVMap *map_stack;

unsigned resolve_new_var(AST_Expression *exp) {
    if(get_map(top_vectorSVMap(map_stack), exp->var_str) != ID_NOT_FOUND) {
        error = 1;
        printf("Redefinition of %.*s\n", exp->var_str.len, exp->var_str.start);
        return ID_NOT_FOUND;
    }
    return insert_map(top_vectorSVMap(map_stack), exp->var_str, map_cnt++);
}

unsigned resolve_exist_var(AST_Expression *exp) {
    unsigned id = ID_NOT_FOUND;
    for(int i = map_stack->size-1;i >= 0;i--) {
        id = get_map(&map_stack->array[i], exp->var_str);
        if(id != ID_NOT_FOUND)
            return id;
    }
    error = 2;
    printf("use of undeclared identifier %.*s\n", exp->var_str.len, exp->var_str.start);
    return ID_NOT_FOUND;
}

#define push_map_stack()\
    SVMap map;\
    init_map(&map);\
    push_vectorSVMap(map_stack, map);\

#define pop_map_stack()\
    free_map(top_vectorSVMap(map_stack));\
    pop_vectorSVMap(map_stack);\

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

void var_resolve_block(AST_Block *block);
void var_resolve_block_item(AST_BlockItem *bi);
void var_resolve_statement(AST_BlockItem *bi) {
    if(bi->type == AST_STATEMENT_IF || bi->type == AST_STATEMENT_IF_ELSE) {
        var_resolve_expression(&bi->if_cond);
        var_resolve_statement(bi->then);
        if(bi->type == AST_STATEMENT_IF_ELSE)
            var_resolve_statement(bi->else_);
    } else if(bi->type == AST_STATEMENT_FOR) {
        push_map_stack();
        var_resolve_block_item(bi->init);
        var_resolve_expression(&bi->loop_cond);
        var_resolve_expression(&bi->loop_it);
        var_resolve_statement(bi->loop_body);
        pop_map_stack();
    } else if(bi->type == AST_STATEMENT_WHILE ||
              bi->type == AST_STATEMENT_DO_WHILE) {
        var_resolve_expression(&bi->loop_cond);
        var_resolve_statement(bi->loop_body);
    } else if(bi->type == AST_STATEMENT_SWITCH) {
        var_resolve_expression(&bi->switch_exp);
        var_resolve_statement(bi->switch_body);
    } else if(bi->type == AST_STATEMENT_COMPOUND) {
        var_resolve_block(&bi->block);
    } else if(bi->type == AST_STATEMENT_EXP ||
              bi->type == AST_STATEMENT_RETURN) {
        var_resolve_expression(&bi->exp);
    }
}

void var_resolve_block_item(AST_BlockItem *bi) {
    switch(bi->type) {
        case AST_STATEMENT_RETURN:
        case AST_STATEMENT_EXP:
        case AST_STATEMENT_NULL:
        case AST_STATEMENT_IF:
        case AST_STATEMENT_IF_ELSE:
        case AST_STATEMENT_COMPOUND:
        case AST_STATEMENT_WHILE:
        case AST_STATEMENT_DO_WHILE:
        case AST_STATEMENT_FOR:
        case AST_STATEMENT_SWITCH:
            var_resolve_statement(bi);
            break;
        case AST_DECLARATION_NO_ASSIGN:
        case AST_DECLARATION_WITH_ASSIGN:
            var_resolve_declaration(bi);
            break;
        case AST_STATEMENT_BREAK:
        case AST_STATEMENT_CONTINUE:
        case AST_STATEMENT_GOTO:
        case AST_LABEL:
        case AST_CASE_LABEL:
        case AST_DEFAULT_LABEL:
            break;
    }
}

void var_resolve_block(AST_Block *block) {
    push_map_stack();

    for(int i = 0;i < block->block_items.size;i++) {
        var_resolve_block_item(&block->block_items.array[i]);
    }

    pop_map_stack();
}

void var_resolve_function(AST_Function *f) {
    var_resolve_block(&f->block);
}

void var_resolve_program(AST_Program *p) {
    map_cnt = 0;

    var_resolve_function(&p->function);
    p->var_cnt = map_cnt;
    p->error = error;
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
    } else if(bi->type == AST_STATEMENT_FOR ||
              bi->type == AST_STATEMENT_WHILE ||
              bi->type == AST_STATEMENT_DO_WHILE) {
        label_resolve_block_item(bi->loop_body, is_label, true);
    } else if(bi->type == AST_STATEMENT_SWITCH) {
        label_resolve_block_item(bi->switch_body, is_label, true);
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
    push_map_stack();

    label_resolve_block(&p->function.block, true);
    label_resolve_block(&p->function.block, false);

    pop_map_stack();

    p->label_cnt = map_cnt;
    p->error = error;
}

static unsigned max_break_label;
static unsigned min_break_label;
void loop_resolve_block(AST_Block *b, unsigned cont_label, unsigned break_label);
void loop_resolve_block_item(AST_BlockItem *bi, unsigned cont_label, unsigned break_label) {
    if(bi->type == AST_STATEMENT_COMPOUND) {
        loop_resolve_block(&bi->block, cont_label, break_label);
    } else if(bi->type == AST_STATEMENT_IF) {
        loop_resolve_block_item(bi->then, cont_label, break_label);
    } else if(bi->type == AST_STATEMENT_IF_ELSE) {
        loop_resolve_block_item(bi->then, cont_label, break_label);
        loop_resolve_block_item(bi->else_, cont_label, break_label);
    } else if(bi->type == AST_STATEMENT_FOR ||
              bi->type == AST_STATEMENT_WHILE ||
              bi->type == AST_STATEMENT_DO_WHILE) {
        bi->loop_id = max_break_label += 2;
        loop_resolve_block_item(bi->loop_body, bi->loop_id, bi->loop_id);
    } else if(bi->type == AST_STATEMENT_SWITCH) {
        bi->switch_id = max_break_label += 1;
        loop_resolve_block_item(bi->switch_body, cont_label, bi->switch_id);
    } else if(bi->type == AST_STATEMENT_BREAK) {
        if(break_label < min_break_label) {
            printf("break statement not in loop or switch statement\n");
            error = 5;
            return;
        }
        bi->exp.type = EXP_VAR_ID;
        bi->exp.var_id = break_label;
    } else if(bi->type == AST_STATEMENT_CONTINUE) {
        if(cont_label < min_break_label) {
            printf("continue statement not in loop statement\n");
            error = 6;
            return;
        }
        bi->exp.type = EXP_VAR_ID;
        bi->exp.var_id = cont_label-1;
    }
}

void loop_resolve_block(AST_Block *b, unsigned cont_label, unsigned break_label) {
    for(unsigned i = 0;i < b->block_items.size;i++) {
        loop_resolve_block_item(&b->block_items.array[i], cont_label, break_label);
    }
}

void loop_resolve_program(AST_Program *p) {
    max_break_label = p->label_cnt+1;
    min_break_label = p->label_cnt+1;
    loop_resolve_block(&p->function.block, p->label_cnt, p->label_cnt);
    p->label_cnt = max_break_label+1;
    p->error = error;
}

int resolve_const_exp(AST_Expression *exp) {
    if(exp->type != EXP_CONSTANT) {
        printf("expected integer constant expression");
        error = 7;
        return -1;
    }
    return exp->constant;
}

static unsigned switch_label_cnt;
static AST_BlockItem *current_switch;

void switch_resolve_block(AST_Block *b);

void switch_resolve_block_item(AST_BlockItem *bi) {
    if(bi->type == AST_STATEMENT_COMPOUND) {
        switch_resolve_block(&bi->block);
    } else if(bi->type == AST_STATEMENT_IF) {
        switch_resolve_block_item(bi->then);
    } else if(bi->type == AST_STATEMENT_IF_ELSE) {
        switch_resolve_block_item(bi->then);
        switch_resolve_block_item(bi->else_);
    } else if(bi->type == AST_STATEMENT_FOR ||
              bi->type == AST_STATEMENT_WHILE ||
              bi->type == AST_STATEMENT_DO_WHILE) {
        bi->loop_id = max_break_label += 2;
        switch_resolve_block_item(bi->loop_body);
    } else if(bi->type == AST_STATEMENT_SWITCH) {
        AST_BlockItem *temp = current_switch;
        current_switch = bi;
        init_vectorIntLabelPair(&bi->labels, 2);
        bi->default_label = 0;
        switch_resolve_block_item(bi->switch_body);
        current_switch = temp;
    } else if(bi->type == AST_CASE_LABEL) {
        if(current_switch == NULL) {
            printf("case statement not in switch statement\n");
            error = 8;
            return;
        }
        int const_int = resolve_const_exp(&bi->exp);if(error)return;
        for(unsigned i = 0;i < current_switch->labels.size;i++) {
            if(const_int == current_switch->labels.array[i].const_int) {
                printf("duplicate case value: %d\n", const_int);
                error = 9;
                return;
            }
        }

        push_vectorIntLabelPair(&current_switch->labels, 
                (IntLabelPair){const_int, switch_label_cnt});

        bi->exp.type = EXP_VAR_ID;
        bi->exp.var_id = switch_label_cnt++;
    } else if(bi->type == AST_DEFAULT_LABEL) {
        if(current_switch == NULL) {
            printf("default statement not in switch statement\n");
            error = 10;
            return;
        }
        if(current_switch->default_label != 0) {
            printf("multiple default labels in one switch");
            error = 11;
            return;
        }

        current_switch->default_label = switch_label_cnt;
        
        bi->exp.type = EXP_VAR_ID;
        bi->exp.var_id = switch_label_cnt++;
    }
}

void switch_resolve_block(AST_Block *b) {
    for(unsigned i = 0;i < b->block_items.size;i++) {
        switch_resolve_block_item(&b->block_items.array[i]);
    }
}

void switch_resolve_program(AST_Program *p) {
    switch_label_cnt = p->label_cnt;
    current_switch = NULL;

    switch_resolve_block(&p->function.block);

    p->label_cnt = switch_label_cnt;
    p->error = error;
}

void resolve_program(AST_Program *p) {
    error = 0;
    VectorSVMap ms;
    init_vectorSVMap(&ms, 2);
    map_stack = &ms;

    var_resolve_program(p);
    if(error)return;
    label_resolve_program(p);
    if(error)return;
    loop_resolve_program(p);
    if(error)return;
    switch_resolve_program(p);
    if(error)return;
}
