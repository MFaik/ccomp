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

TAC_Val tac_expression(AST_Expression exp, VectorTAC_Ins *v);
TAC_Val tac_unary(AST_Expression exp, TAC_Ins_Type type, VectorTAC_Ins *v) {
    TAC_Ins ins;
    ins.type = type;
    ins.src = tac_expression(*exp.unary_exp, v);
    ins.unary_dst = generate_new_var();
    insert_vectorTAC_Ins(v, ins);
    return ins.unary_dst;
}

TAC_Val tac_binary(AST_Expression exp, TAC_Ins_Type type, VectorTAC_Ins *v) {
    TAC_Ins ins;
    ins.type = type;
    ins.src1 = tac_expression(*exp.left_exp, v);
    ins.src2 = tac_expression(*exp.right_exp, v);
    ins.binary_dst = generate_new_var();
    insert_vectorTAC_Ins(v, ins);
    return ins.binary_dst;
}

TAC_Val tac_binary_short_circuit(AST_Expression exp, bool jump_on_true, VectorTAC_Ins *v) {
    TAC_Val skip_label = generate_new_label();
    TAC_Val end_label = generate_new_label();
    TAC_Val ret = generate_new_var();

    TAC_Val left = tac_expression(*exp.left_exp, v);

    TAC_Ins jmp;
    jmp.type = jump_on_true ? TAC_INS_JMP_IF_NOT_ZERO : TAC_INS_JMP_IF_ZERO;
    jmp.condition = left;
    jmp.target = skip_label;
    insert_vectorTAC_Ins(v, jmp);

    TAC_Val right = tac_expression(*exp.right_exp, v);

    jmp.condition = right;
    insert_vectorTAC_Ins(v, jmp);

    TAC_Ins copy;
    copy.type = TAC_INS_COPY;
    copy.src = generate_constant(!jump_on_true);
    copy.unary_dst = ret;
    insert_vectorTAC_Ins(v, copy);

    jmp.type = TAC_INS_JMP;
    jmp.target = end_label;
    insert_vectorTAC_Ins(v, jmp);

    TAC_Ins label;
    label.type = TAC_INS_LABEL;
    label.label = skip_label;
    insert_vectorTAC_Ins(v, label);

    copy.type = TAC_INS_COPY;
    copy.src = generate_constant(jump_on_true);
    copy.unary_dst = ret;
    insert_vectorTAC_Ins(v, copy);
    
    label.label = end_label;
    insert_vectorTAC_Ins(v, label);

    return ret;
}

TAC_Val tac_expression(AST_Expression exp, VectorTAC_Ins *v) {
    TAC_Val ret;
    switch(exp.type) {
        case EXP_CONSTANT:
            ret.type = TAC_VAL_CONSTANT;
            ret.constant = exp.constant;
            return ret;
        //unary expressions
        case EXP_UNARY_NEG:
            return tac_unary(exp, TAC_INS_UNARY_NEG, v);
        case EXP_UNARY_COMPLEMENT:
            return tac_unary(exp, TAC_INS_UNARY_COMPLEMENT, v);
        case EXP_UNARY_LOGICAL_NOT:
            return tac_unary(exp, TAC_INS_UNARY_LOGICAL_NOT, v);
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
        case EXP_EQUAL:
            return tac_binary(exp, TAC_INS_BINARY_EQUAL, v);
        case EXP_NOT_EQUAL:
            return tac_binary(exp, TAC_INS_BINARY_NOT_EQUAL, v);
        case EXP_LESS_THAN:
            return tac_binary(exp, TAC_INS_BINARY_LESS_THAN, v);
        case EXP_GREATER_THAN:
            return tac_binary(exp, TAC_INS_BINARY_GREATER_THAN, v);
        case EXP_LESS_OR_EQUAL:
            return tac_binary(exp, TAC_INS_BINARY_LESS_OR_EQUAL, v);
        case EXP_GREATER_OR_EQUAL:
            return tac_binary(exp, TAC_INS_BINARY_GREATER_OR_EQUAL, v);
    }
}

TAC_Function tac_function(AST_Function function) {
    TAC_Function ret;
    ret.name = function.name.str;
    init_vectorTAC_Ins(&ret.instructions, 2);
    TAC_Ins ret_ins;
    ret_ins.type = TAC_INS_RETURN;
    ret_ins.ret = tac_expression(function.statement.ret, &ret.instructions);
    insert_vectorTAC_Ins(&ret.instructions, ret_ins);
    ret.var_cnt = generate_new_var().var;
    return ret;
}

TAC_Program emit_tacky(AST_Program ast_program) {
    var_counter = 0;
    label_counter = 0;

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
