#include "tacky.h"
vector_body(TAC_Ins);

#include <stdio.h>

static unsigned counter = 0;

TAC_Val generate_new_var() {
    TAC_Val ret;
    ret.type = TAC_VAL_VAR;
    ret.var = counter++;
    return ret;
}

TAC_Val tac_expression(AST_Expression exp, VectorTAC_Ins *v);
TAC_Val tac_unary(AST_Expression exp, TAC_Ins_Type type, VectorTAC_Ins *v) {
    TAC_Ins ins;
    ins.type = type;
    ins.src = tac_expression(*exp.unary_exp, v);
    ins.dst = generate_new_var();
    insert_vectorTAC_Ins(v, ins);
    return ins.dst;
}

TAC_Val tac_expression(AST_Expression exp, VectorTAC_Ins *v) {
    TAC_Val ret;
    switch(exp.type) {
        case EXP_CONSTANT:
            ret.type = TAC_VAL_CONSTANT;
            ret.constant = exp.constant;
            return ret;
        case EXP_UNARY_NEG:
            return tac_unary(exp, TAC_INS_UNARY_NEG, v);
        case EXP_UNARY_COMPLEMENT:
            return tac_unary(exp, TAC_INS_UNARY_COMPLEMENT, v);
    }
}

TAC_Function tac_function(AST_Function function) {
    counter = 0;
    TAC_Function ret;
    ret.name = function.name.str;
    init_vectorTAC_Ins(&ret.instructions, 2);
    TAC_Ins ret_ins;
    ret_ins.type = TAC_INS_RETURN;
    ret_ins.ret = tac_expression(function.statement.ret, &ret.instructions);
    insert_vectorTAC_Ins(&ret.instructions, ret_ins);
    ret.var_cnt = counter;
    return ret;
}

TAC_Program emit_tacky(AST_Program ast_program) {
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
void pretty_print_tacky_program(TAC_Program program) {
    printf("%.*s\n", program.function.name.len, program.function.name.start);
    for(int i = 0;i < program.function.instructions.size;i++) {
        TAC_Ins ins = program.function.instructions.array[i];
        switch(ins.type) {
            case TAC_INS_RETURN:
                printf("return(");
                pretty_print_val(ins.ret);
                printf(")\n");
                break;
            case TAC_INS_UNARY_NEG:
                pretty_print_val(ins.dst);
                printf(" = -(");
                pretty_print_val(ins.src);
                printf(")\n");
                break;
            case TAC_INS_UNARY_COMPLEMENT:
                pretty_print_val(ins.dst);
                printf(" = ~(");
                pretty_print_val(ins.src);
                printf(")\n");
                break;
        }
    }
}
