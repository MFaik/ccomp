#include "tacky.h"
#include "parser.h"
vector_body(TAC_Ins);

#include <stdio.h>
#include <stdbool.h>

static unsigned var_counter = 0;
static unsigned label_counter = 0;

TAC_Val generate_constant(long l) {
    TAC_Val ret;
    ret.type = TAC_VAL_CONSTANT;
    ret.constant = l;
    return ret;
}

TAC_Val generate_new_var() {
    TAC_Val ret;
    ret.type = TAC_VAL_VAR;
    ret.var = var_counter++;
    return ret;
}

TAC_Val generate_new_label() {
    TAC_Val ret;
    ret.type = TAC_VAL_VAR;
    ret.var = label_counter++;
    return ret;
}

TAC_Val generate_var(unsigned u) {
    TAC_Val ret;
    ret.type = TAC_VAL_VAR;
    ret.var = u;
    return ret;
}

TAC_Val tac_copy(TAC_Val src, TAC_Val dst, VectorTAC_Ins *v) {
    TAC_Ins copy;
    copy.type = TAC_INS_COPY;
    copy.src = src;
    copy.unary_dst = dst;
    push_vectorTAC_Ins(v, copy);
    return dst;
}

void tac_label(TAC_Val label, VectorTAC_Ins *v) {
    TAC_Ins ins;
    ins.type = TAC_INS_LABEL;
    ins.label = label;
    push_vectorTAC_Ins(v, ins);
}

void tac_jump_cond(bool jump_on_true, TAC_Val condition, TAC_Val target, VectorTAC_Ins *v) {
    TAC_Ins jmp;
    jmp.type = jump_on_true ? TAC_INS_JMP_IF_NOT_ZERO : TAC_INS_JMP_IF_ZERO;
    jmp.condition = condition;
    jmp.target = target;
    push_vectorTAC_Ins(v, jmp);
}

void tac_jump(TAC_Val target, VectorTAC_Ins *v) {
    TAC_Ins jmp;
    jmp.type = TAC_INS_JMP;
    jmp.target = target;
    push_vectorTAC_Ins(v, jmp);
}

TAC_Val tac_expression(AST_Expression exp, VectorTAC_Ins *v);
TAC_Val tac_unary(AST_Expression exp, TAC_Ins_Type type, VectorTAC_Ins *v) {
    TAC_Ins ins;
    ins.type = type;
    ins.src = tac_expression(*exp.unary_exp, v);
    ins.unary_dst = generate_new_var();
    push_vectorTAC_Ins(v, ins);
    return ins.unary_dst;
}

TAC_Val tac_binary(AST_Expression exp, TAC_Ins_Type type, VectorTAC_Ins *v) {
    TAC_Ins ins;
    ins.type = type;
    ins.src1 = tac_expression(*exp.left_exp, v);
    ins.src2 = tac_expression(*exp.right_exp, v);
    ins.binary_dst = generate_new_var();
    push_vectorTAC_Ins(v, ins);
    return ins.binary_dst;
}

TAC_Val tac_binary_short_circuit(AST_Expression exp, bool jump_on_true, VectorTAC_Ins *v) {
    TAC_Val skip_label = generate_new_label();
    TAC_Val end_label = generate_new_label();
    TAC_Val ret = generate_new_var();

    TAC_Val left = tac_expression(*exp.left_exp, v);
    tac_jump_cond(jump_on_true, left, skip_label, v);

    TAC_Val right = tac_expression(*exp.right_exp, v);
    tac_jump_cond(jump_on_true, right, skip_label, v);

    tac_copy(generate_constant(!jump_on_true), ret, v);
    tac_jump(end_label, v);

    tac_label(skip_label, v);
    tac_copy(generate_constant(jump_on_true), ret, v);
    
    tac_label(end_label, v);

    return ret;
}

TAC_Val tac_binary_assign(TAC_Ins_Type type, AST_Expression exp, VectorTAC_Ins *v) {
    return tac_copy(tac_binary(exp, type, v), 
            tac_expression(*exp.left_exp, v) ,v);
}

TAC_Val tac_inc_dec(bool inc, bool pre, AST_Expression exp, VectorTAC_Ins *v) {
    TAC_Val src = tac_expression(*exp.unary_exp, v);
    TAC_Val dst = generate_new_var();
    TAC_Ins inc_or_dec;
    inc_or_dec.type = inc ? TAC_INS_BINARY_ADD : TAC_INS_BINARY_SUB;
    inc_or_dec.src1 = src;
    inc_or_dec.src2 = generate_constant(1);
    inc_or_dec.binary_dst = src;
    
    if(pre) {
        push_vectorTAC_Ins(v, inc_or_dec);
        tac_copy(src, dst, v);
    } else {
        tac_copy(src, dst, v);
        push_vectorTAC_Ins(v, inc_or_dec);
    }
    return dst;
}

TAC_Val tac_conditional(AST_Expression exp, VectorTAC_Ins *v) {
    TAC_Val ret = generate_new_var();
    TAC_Val false_label = generate_new_label();
    TAC_Val end_label = generate_new_label();

    TAC_Val cond = tac_expression(*exp.cond, v);
    
    tac_jump_cond(false, cond, false_label, v);

    TAC_Val true_exp = tac_expression(*exp.true_exp, v);
    tac_copy(true_exp, ret, v);
    tac_jump(end_label, v);

    tac_label(false_label, v);
    TAC_Val false_exp = tac_expression(*exp.false_exp, v);
    tac_copy(false_exp, ret, v);

    tac_label(end_label, v);
    return ret;
}

TAC_Val tac_expression(AST_Expression exp, VectorTAC_Ins *v) {
    TAC_Val ret;
    switch(exp.type) {
        case EXP_CONSTANT:
            ret.type = TAC_VAL_CONSTANT;
            ret.constant = exp.constant;
            return ret;
        case EXP_VAR_ID:
            ret.type = TAC_VAL_VAR;
            ret.var = exp.var_id;
            return ret;
        case EXP_VAR_STR:
            printf("internal error");
            exit(2);
        case EXP_CONDITIONAL:
            return tac_conditional(exp, v);
        //unary expressions
        case EXP_UNARY_NEG:
            return tac_unary(exp, TAC_INS_UNARY_NEG, v);
        case EXP_UNARY_COMPLEMENT:
            return tac_unary(exp, TAC_INS_UNARY_COMPLEMENT, v);
        case EXP_UNARY_LOGICAL_NOT:
            return tac_unary(exp, TAC_INS_UNARY_LOGICAL_NOT, v);
        case EXP_UNARY_PRE_INCREMENT:
            return tac_inc_dec(true, true, exp, v);
        case EXP_UNARY_PRE_DECREMENT:
            return tac_inc_dec(false, true, exp, v);
        case EXP_UNARY_POST_INCREMENT:
            return tac_inc_dec(true, false, exp, v);
        case EXP_UNARY_POST_DECREMENT:
            return tac_inc_dec(false, false, exp, v);
        //binary expressions
        case EXP_BINARY_ADD:
            return tac_binary(exp, TAC_INS_BINARY_ADD, v);
        case EXP_BINARY_SUB:
            return tac_binary(exp, TAC_INS_BINARY_SUB, v);
        case EXP_BINARY_MUL:
            return tac_binary(exp, TAC_INS_BINARY_MUL, v);
        case EXP_BINARY_DIV:
            return tac_binary(exp, TAC_INS_BINARY_DIV, v);
        case EXP_BINARY_REMAINDER:
            return tac_binary(exp, TAC_INS_BINARY_REMAINDER, v);
        case EXP_BINARY_BITWISE_AND:
            return tac_binary(exp, TAC_INS_BINARY_BITWISE_AND, v);
        case EXP_BINARY_BITWISE_OR:
            return tac_binary(exp, TAC_INS_BINARY_BITWISE_OR, v);
        case EXP_BINARY_BITWISE_XOR:
            return tac_binary(exp, TAC_INS_BINARY_BITWISE_XOR, v);
        case EXP_BINARY_LEFT_SHIFT:
            return tac_binary(exp, TAC_INS_BINARY_LEFT_SHIFT, v);
        case EXP_BINARY_RIGHT_SHIFT:
            return tac_binary(exp, TAC_INS_BINARY_LEFT_SHIFT, v);
        case EXP_BINARY_LOGICAL_AND:
            return tac_binary_short_circuit(exp, 0, v);
        case EXP_BINARY_LOGICAL_OR:
            return tac_binary_short_circuit(exp, 1, v);
        case EXP_BINARY_EQUAL:
            return tac_binary(exp, TAC_INS_BINARY_EQUAL, v);
        case EXP_BINARY_NOT_EQUAL:
            return tac_binary(exp, TAC_INS_BINARY_NOT_EQUAL, v);
        case EXP_BINARY_LESS_THAN:
            return tac_binary(exp, TAC_INS_BINARY_LESS_THAN, v);
        case EXP_BINARY_GREATER_THAN:
            return tac_binary(exp, TAC_INS_BINARY_GREATER_THAN, v);
        case EXP_BINARY_LESS_OR_EQUAL:
            return tac_binary(exp, TAC_INS_BINARY_LESS_OR_EQUAL, v);
        case EXP_BINARY_GREATER_OR_EQUAL:
            return tac_binary(exp, TAC_INS_BINARY_GREATER_OR_EQUAL, v);
        case EXP_BINARY_ASSIGN:
            return tac_copy(tac_expression(*exp.right_exp, v), 
                            tac_expression(*exp.left_exp, v), v);
        case EXP_BINARY_ADD_ASSIGN:
            return tac_binary_assign(TAC_INS_BINARY_ADD, exp, v);
        case EXP_BINARY_SUB_ASSIGN:
            return tac_binary_assign(TAC_INS_BINARY_SUB, exp, v);
        case EXP_BINARY_MUL_ASSIGN:
            return tac_binary_assign(TAC_INS_BINARY_MUL, exp, v);
        case EXP_BINARY_DIV_ASSIGN:
            return tac_binary_assign(TAC_INS_BINARY_DIV, exp, v);
        case EXP_BINARY_REMAINDER_ASSIGN:
            return tac_binary_assign(TAC_INS_BINARY_REMAINDER, exp, v);
        case EXP_BINARY_AND_ASSIGN:
            return tac_binary_assign(TAC_INS_BINARY_BITWISE_AND, exp, v);
        case EXP_BINARY_OR_ASSIGN:
            return tac_binary_assign(TAC_INS_BINARY_BITWISE_OR, exp, v);
        case EXP_BINARY_XOR_ASSIGN:
            return tac_binary_assign(TAC_INS_BINARY_BITWISE_XOR, exp, v);
        case EXP_BINARY_LEFT_SHIFT_ASSIGN:
            return tac_binary_assign(TAC_INS_BINARY_LEFT_SHIFT, exp, v);
        case EXP_BINARY_RIGHT_SHIFT_ASSIGN:
            return tac_binary_assign(TAC_INS_BINARY_RIGHT_SHIFT, exp, v);
    }
}

void tac_statement(AST_BlockItem bi, VectorTAC_Ins *v) {
    switch(bi.type) {
        case AST_STATEMENT_COMPOUND:
            for(int i = 0;i < bi.block.block_items.size;i++) {
                tac_statement(bi.block.block_items.array[i], v);
            }
            break;
        case AST_STATEMENT_RETURN:
        {
            TAC_Ins ret_ins;
            ret_ins.type = TAC_INS_RETURN;
            ret_ins.ret = tac_expression(bi.exp, v);
            push_vectorTAC_Ins(v, ret_ins);
            break;
        }
        case AST_STATEMENT_EXP:
            tac_expression(bi.exp, v);
            break;
        case AST_STATEMENT_IF:
        case AST_STATEMENT_IF_ELSE:
        {
            TAC_Val else_label = generate_new_label();
            TAC_Val end_label;
            if(bi.type == AST_STATEMENT_IF_ELSE)
                end_label = generate_new_label();

            TAC_Val cond = tac_expression(bi.if_cond, v);
            tac_jump_cond(false, cond, else_label, v);

            tac_statement(*bi.then, v);

            if(bi.type != AST_STATEMENT_IF_ELSE) {
                tac_label(else_label, v);
            } else {
                tac_jump(end_label, v);

                tac_label(else_label, v);
                tac_statement(*bi.else_, v);

                tac_label(end_label, v);
            }
            break;
        }
        case AST_STATEMENT_WHILE:
        {
            TAC_Val cont_label = generate_var(bi.loop_id-1);
            TAC_Val break_label = generate_var(bi.loop_id);
            
            tac_label(cont_label, v);

            TAC_Val cond = tac_expression(bi.loop_cond, v);
            tac_jump_cond(false, cond, break_label, v);

            tac_statement(*bi.loop_body, v);

            tac_jump(cont_label, v);

            tac_label(break_label, v);
            break;
        }
        case AST_STATEMENT_DO_WHILE:
        {
            TAC_Val cont_label = generate_var(bi.loop_id-1);
            TAC_Val break_label = generate_var(bi.loop_id);
            TAC_Val start_label = generate_new_label();

            tac_label(start_label, v);

            tac_statement(*bi.loop_body, v);

            tac_label(cont_label, v);
            TAC_Val cond = tac_expression(bi.loop_cond, v);
            tac_jump_cond(true, cond, start_label, v);

            tac_label(break_label, v);
            
            break;
        }
        case AST_STATEMENT_FOR:
        {
            TAC_Val cont_label = generate_var(bi.loop_id-1);
            TAC_Val break_label = generate_var(bi.loop_id);
            TAC_Val start_label = generate_new_label();
            
            tac_statement(*bi.init, v);

            tac_label(start_label, v);
            TAC_Val cond = tac_expression(bi.loop_cond, v);
            tac_jump_cond(false, cond, break_label, v);

            tac_statement(*bi.loop_body, v);
            tac_label(cont_label, v);
            tac_expression(bi.loop_it, v);

            tac_jump(start_label, v);

            tac_label(break_label, v);
            break;
        }
        case AST_STATEMENT_BREAK:
        case AST_STATEMENT_CONTINUE:
            tac_jump(generate_var(bi.exp.var_id), v);
            break;
        case AST_DECLARATION_WITH_ASSIGN:
            tac_copy(tac_expression(bi.assign_exp, v), 
                     tac_expression(bi.var, v), v);
            break;
        case AST_STATEMENT_GOTO:
            tac_jump(tac_expression(bi.exp, v), v);
            break;
        case AST_LABEL:
            tac_label(tac_expression(bi.exp, v), v);
            break;
        case AST_STATEMENT_NULL:
        case AST_DECLARATION_NO_ASSIGN:
            break;
    }
}

TAC_Function tac_function(AST_Function function) {
    TAC_Function ret;
    ret.name = function.name.str;
    init_vectorTAC_Ins(&ret.instructions, function.block.block_items.size*2);
    for(int i = 0;i < function.block.block_items.size;i++) {
        tac_statement(function.block.block_items.array[i], &ret.instructions);
    }
    ret.var_cnt = var_counter+1;
    return ret;
}

TAC_Program emit_tacky(AST_Program ast_program) {
    var_counter = ast_program.var_cnt;
    label_counter = ast_program.label_cnt+1;

    TAC_Program ret;
    ret.function = tac_function(ast_program.function);
    return ret;
}

void pretty_print_val(TAC_Val val) {
    switch(val.type) {
        case TAC_VAL_CONSTANT:
            printf("%ld", val.constant);
            break;
        case TAC_VAL_VAR:
            printf("$%d", val.var);
            break;
    }
}

void pretty_print_unary(TAC_Ins ins, const char* op) {
    pretty_print_val(ins.unary_dst);
    printf(" = %s", op);
    pretty_print_val(ins.src);
    printf("\n");
}

void pretty_print_binary(TAC_Ins ins, const char* op) {
    pretty_print_val(ins.binary_dst);
    printf(" = ");
    pretty_print_val(ins.src1);
    printf(" %s ", op);
    pretty_print_val(ins.src2);
    printf("\n");
}

void pretty_print_tacky_program(TAC_Program program) {
    printf("%.*s\n", program.function.name.len, program.function.name.start);
    for(int i = 0;i < program.function.instructions.size;i++) {
        TAC_Ins ins = program.function.instructions.array[i];
        switch(ins.type) {
            case TAC_INS_RETURN:
                printf("return ");
                pretty_print_val(ins.ret);
                printf("\n");
                break;
            case TAC_INS_UNARY_NEG:
                pretty_print_unary(ins, "-");
                break;
            case TAC_INS_UNARY_COMPLEMENT:
                pretty_print_unary(ins, "~");
                break;
            case TAC_INS_UNARY_LOGICAL_NOT:
                pretty_print_unary(ins, "!");
                break;
            case TAC_INS_COPY:
                pretty_print_unary(ins, "");
                break;
            case TAC_INS_BINARY_ADD:
                pretty_print_binary(ins, "+");
                break;
            case TAC_INS_BINARY_SUB:
                pretty_print_binary(ins, "-");
                break;
            case TAC_INS_BINARY_MUL:
                pretty_print_binary(ins, "*");
                break;
            case TAC_INS_BINARY_DIV:
                pretty_print_binary(ins, "/");
                break;
            case TAC_INS_BINARY_REMAINDER:
                pretty_print_binary(ins, "%");
                break;
            case TAC_INS_BINARY_BITWISE_AND:
                pretty_print_binary(ins, "&");
                break;
            case TAC_INS_BINARY_BITWISE_OR:
                pretty_print_binary(ins, "|");
                break;
            case TAC_INS_BINARY_BITWISE_XOR:
                pretty_print_binary(ins, "^");
                break;
            case TAC_INS_BINARY_LEFT_SHIFT:
                pretty_print_binary(ins, "<<");
                break;
            case TAC_INS_BINARY_RIGHT_SHIFT:
                pretty_print_binary(ins, ">>");
                break;
            case TAC_INS_BINARY_EQUAL:
                pretty_print_binary(ins, "==");
                break;
            case TAC_INS_BINARY_NOT_EQUAL:
                pretty_print_binary(ins, "!=");
                break;
            case TAC_INS_BINARY_GREATER_THAN:
                pretty_print_binary(ins, ">");
                break;
            case TAC_INS_BINARY_LESS_THAN:
                pretty_print_binary(ins, "<");
                break;
            case TAC_INS_BINARY_GREATER_OR_EQUAL:
                pretty_print_binary(ins, ">=");
                break;
            case TAC_INS_BINARY_LESS_OR_EQUAL:
                pretty_print_binary(ins, "<=");
                break;
            case TAC_INS_JMP:
                printf("jmp ");
                pretty_print_val(ins.target);
                printf("\n");
                break;
            case TAC_INS_JMP_IF_ZERO:
                printf("if ");
                pretty_print_val(ins.condition);
                printf(" == 0 jmp ");
                pretty_print_val(ins.target);
                printf("\n");
                break;
            case TAC_INS_JMP_IF_NOT_ZERO:
                printf("if ");
                pretty_print_val(ins.condition);
                printf(" != 0 jmp ");
                pretty_print_val(ins.target);
                printf("\n");
                break;
            case TAC_INS_LABEL:
                printf("label ");
                pretty_print_val(ins.label);
                printf("\n");
                break;
        }
    }
}
