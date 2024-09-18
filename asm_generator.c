#include "asm_generator.h"

#include "vector.h"

vector_body(ASM_Ins);

ASM_Operand assemble_imm(int constant) {
    return (ASM_Operand){OP_IMM, constant};
}

ASM_Operand assemble_operand(TAC_Val val) {
    ASM_Operand ret;
    switch(val.type) {
        case TAC_VAL_CONSTANT:
            return assemble_imm(val.constant);
        case TAC_VAL_VAR:
            ret.type = OP_PSEUDO;
            ret.pseudo = val.var;
            return ret;
    }
}

void assemble_mov(ASM_Operand src, ASM_Operand dst, VectorASM_Ins *v) {
    ASM_Ins mov;
    mov.type = ASM_INS_MOV;
    if(src.type == OP_PSEUDO && dst.type == OP_PSEUDO) {
        mov.src = src;
        mov.dst = (ASM_Operand){OP_REG_R10};
        insert_vectorASM_Ins(v, mov);
        
        mov.src = (ASM_Operand){OP_REG_R10};
        mov.dst = dst;
        insert_vectorASM_Ins(v, mov);
    } else {
        mov.src = src;
        mov.dst = dst;
        insert_vectorASM_Ins(v, mov);
    }
}

void assemble_unary(ASM_InsType type, TAC_Ins tac_ins, VectorASM_Ins *v) {
    ASM_Operand src = assemble_operand(tac_ins.src);
    ASM_Operand dst = assemble_operand(tac_ins.dst);

    assemble_mov(src, dst, v);

    ASM_Ins ins;
    ins.type = type;
    ins.op = dst;
    insert_vectorASM_Ins(v, ins);
}

void assemble_instruction(TAC_Ins tac_ins, VectorASM_Ins *v) {
    ASM_Ins asm_ins;
    switch(tac_ins.type) {
        case TAC_INS_RETURN:{
            ASM_Operand src = assemble_operand(tac_ins.ret);
            ASM_Operand dst = (ASM_Operand){OP_REG_AX};
            assemble_mov(src, dst, v);
            insert_vectorASM_Ins(v, (ASM_Ins){ASM_INS_RET});
            break;
        }
        case TAC_INS_UNARY_NEG:
            assemble_unary(ASM_INS_UNARY_NEG, tac_ins, v);
            break;
        case TAC_INS_UNARY_COMPLEMENT:
            assemble_unary(ASM_INS_UNARY_COMPLEMENT, tac_ins, v);
            break;
    }
}

ASM_Function assemble_function(TAC_Function function) {
    ASM_Function ret;
    ret.name = function.name;
    init_vectorASM_Ins(&ret.instructions, 2);
    ASM_Ins a;
    a.type = ASM_INS_ALLOCATE;
    a.alloc = function.var_cnt;
    insert_vectorASM_Ins(&ret.instructions, a);
    for(int i = 0;i < function.instructions.size;i++) {
        assemble_instruction(function.instructions.array[i], &ret.instructions);
    }
    return ret;
}

ASM_Program assemble_program(TAC_Program program) {
    ASM_Program ret;
    ret.function = assemble_function(program.function);
    return ret;
}
