#include "asm_generator.h"
vector_body(ASM_Ins);

#include <stdbool.h>

ASM_Operand assemble_constant(int constant) {
    return (ASM_Operand){OP_IMM, constant};
}

ASM_Operand assemble_operand(TAC_Val val) {
    ASM_Operand ret;
    switch(val.type) {
        case TAC_VAL_CONSTANT:
            return assemble_constant(val.constant);
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
        push_vectorASM_Ins(v, mov);
        
        mov.src = (ASM_Operand){OP_REG_R10};
        mov.dst = dst;
        push_vectorASM_Ins(v, mov);
    } else {
        mov.src = src;
        mov.dst = dst;
        push_vectorASM_Ins(v, mov);
    }
}

void assemble_unary(ASM_InsType type, TAC_Ins tac_ins, VectorASM_Ins *v) {
    ASM_Operand src = assemble_operand(tac_ins.src);
    ASM_Operand dst = assemble_operand(tac_ins.unary_dst);

    assemble_mov(src, dst, v);

    ASM_Ins ins;
    ins.type = type;
    ins.op = dst;
    push_vectorASM_Ins(v, ins);
}

void assemble_setcc(ASM_Cond cond, ASM_Operand dst, VectorASM_Ins *v) {
    ASM_Ins mov;
    mov.type = ASM_INS_MOV;
    mov.src = assemble_constant(0);
    mov.dst = dst;
    push_vectorASM_Ins(v, mov);

    ASM_Ins set;
    set.type = ASM_INS_SETCC;
    set.cond = cond;
    set.set_op = dst;
    push_vectorASM_Ins(v, set);
}

ASM_Operand MMtoRM(ASM_Operand a, ASM_Operand b, VectorASM_Ins *v) {
    if(a.type == OP_PSEUDO && b.type == OP_PSEUDO) {
        assemble_mov(a, (ASM_Operand){OP_REG_R10}, v);
        return (ASM_Operand){OP_REG_R10};
    } else {
        return a;
    }
}

ASM_Operand IItoRI(ASM_Operand a, ASM_Operand b, VectorASM_Ins *v) {
    if(a.type == OP_IMM && b.type == OP_IMM) {
        assemble_mov(a, (ASM_Operand){OP_REG_R10}, v);
        return (ASM_Operand){OP_REG_R10};
    } else {
        return a;
    }
}

void assemble_unary_logical_not(TAC_Ins tac_ins, VectorASM_Ins *v) {
    ASM_Operand src = assemble_operand(tac_ins.src);
    ASM_Operand dst = assemble_operand(tac_ins.unary_dst);

    ASM_Ins cmp;
    cmp.type = ASM_INS_CMP;
    cmp.src = assemble_constant(0);
    cmp.dst = IItoRI(src, cmp.src, v);
    push_vectorASM_Ins(v, cmp);

    assemble_setcc(ASM_COND_E, dst, v);
}

void assemble_binary(ASM_InsType type, TAC_Ins tac_ins, VectorASM_Ins *v) {
    ASM_Operand src1 = assemble_operand(tac_ins.src1);
    ASM_Operand src2 = assemble_operand(tac_ins.src2);
    ASM_Operand dst = assemble_operand(tac_ins.binary_dst);
    ASM_Ins ins;
    ins.type = type;
    if(type == ASM_INS_BINARY_MUL) {
        assemble_mov(src1, (ASM_Operand){OP_REG_R10}, v);
        assemble_mov(src2, (ASM_Operand){OP_REG_R11}, v);
        ins.src = (ASM_Operand){OP_REG_R10};
        ins.dst = (ASM_Operand){OP_REG_R11};
        push_vectorASM_Ins(v, ins);
        assemble_mov((ASM_Operand){OP_REG_R11}, dst, v);
    } else if(type == ASM_INS_BINARY_LEFT_SHIFT || type == ASM_INS_BINARY_RIGHT_SHIFT) {
        assemble_mov(src1, dst, v);
        ins.dst = dst;
        assemble_mov(src2, (ASM_Operand){OP_REG_CX}, v);
        ins.src = (ASM_Operand){OP_REG_CX};
        push_vectorASM_Ins(v, ins);
    } else {
        assemble_mov(src1, dst, v);
        ins.dst = dst;
        ins.src = MMtoRM(src2, dst, v);
        push_vectorASM_Ins(v, ins);
    }
}

void assemble_div(bool is_remainder, TAC_Ins tac_ins, VectorASM_Ins *v) {
    ASM_Operand src1 = assemble_operand(tac_ins.src1);
    ASM_Operand src2 = assemble_operand(tac_ins.src2);
    ASM_Operand dst = assemble_operand(tac_ins.unary_dst);
    assemble_mov(src1, (ASM_Operand){OP_REG_AX}, v);
    push_vectorASM_Ins(v, (ASM_Ins){ASM_INS_CDQ});
    ASM_Ins ins;
    ins.type = ASM_INS_UNARY_IDIV;
    if(src2.type == OP_IMM) {
        assemble_mov(src2, (ASM_Operand){OP_REG_R10}, v);
        ins.op = (ASM_Operand){OP_REG_R10};
    } else {
        ins.op = src2;
    }
    push_vectorASM_Ins(v, ins);
    if(is_remainder) {
        assemble_mov((ASM_Operand){OP_REG_DX}, dst, v);
    } else {
        assemble_mov((ASM_Operand){OP_REG_AX}, dst, v);
    }
}

void assemble_jmp(bool jump_on_true, TAC_Ins tac_ins, VectorASM_Ins *v) {
    ASM_Ins cmp;
    cmp.type = ASM_INS_CMP;
    cmp.src = assemble_constant(0);
    cmp.dst = IItoRI(assemble_operand(tac_ins.condition), cmp.src, v);
    push_vectorASM_Ins(v, cmp);

    ASM_Ins jmp;
    jmp.type = ASM_INS_JMPCC;
    jmp.cond = jump_on_true ? ASM_COND_NE : ASM_COND_E;
    jmp.label = tac_ins.target.var;
    push_vectorASM_Ins(v, jmp);
}

void assemble_comp(ASM_Cond cond, TAC_Ins tac_ins, VectorASM_Ins *v) {
    ASM_Operand src1 = assemble_operand(tac_ins.src1);
    ASM_Operand src2 = assemble_operand(tac_ins.src2);
    ASM_Operand dst = assemble_operand(tac_ins.binary_dst);

    ASM_Ins cmp;
    cmp.type = ASM_INS_CMP;
    cmp.src = MMtoRM(src1, src2, v);
    cmp.dst = IItoRI(src2, src1, v);

    push_vectorASM_Ins(v, cmp);
    
    assemble_setcc(cond, dst, v);
}

void assemble_instruction(TAC_Ins tac_ins, VectorASM_Ins *v) {
    switch(tac_ins.type) {
        case TAC_INS_RETURN:
        {
            ASM_Operand src = assemble_operand(tac_ins.ret);
            ASM_Operand dst = (ASM_Operand){OP_REG_AX};
            assemble_mov(src, dst, v);
            push_vectorASM_Ins(v, (ASM_Ins){ASM_INS_RET});
            break;
        }
        case TAC_INS_UNARY_NEG:
            assemble_unary(ASM_INS_UNARY_NEG, tac_ins, v);
            break;
        case TAC_INS_UNARY_COMPLEMENT:
            assemble_unary(ASM_INS_UNARY_COMPLEMENT, tac_ins, v);
            break;
        case TAC_INS_UNARY_LOGICAL_NOT:
            assemble_unary_logical_not(tac_ins, v);
            break;
        case TAC_INS_BINARY_ADD:
            assemble_binary(ASM_INS_BINARY_ADD, tac_ins, v);
            break;
        case TAC_INS_BINARY_SUB:
            assemble_binary(ASM_INS_BINARY_SUB, tac_ins, v);
            break;
        case TAC_INS_BINARY_MUL:
            assemble_binary(ASM_INS_BINARY_MUL, tac_ins, v);
            break;
        case TAC_INS_BINARY_BITWISE_AND:
            assemble_binary(ASM_INS_BINARY_BITWISE_AND, tac_ins, v);
            break;
        case TAC_INS_BINARY_BITWISE_OR:
            assemble_binary(ASM_INS_BINARY_BITWISE_OR, tac_ins, v);
            break;
        case TAC_INS_BINARY_BITWISE_XOR:
            assemble_binary(ASM_INS_BINARY_BITWISE_XOR, tac_ins, v);
            break;
        case TAC_INS_BINARY_LEFT_SHIFT:
            assemble_binary(ASM_INS_BINARY_LEFT_SHIFT, tac_ins, v);
            break;
        case TAC_INS_BINARY_RIGHT_SHIFT:
            assemble_binary(ASM_INS_BINARY_RIGHT_SHIFT, tac_ins, v);
            break;
        case TAC_INS_BINARY_DIV:
            assemble_div(true, tac_ins, v);
            break;
        case TAC_INS_BINARY_REMAINDER:
            assemble_div(true, tac_ins, v);
            break;
        case TAC_INS_JMP:
        {
            ASM_Ins jmp;
            jmp.type = ASM_INS_JMP;
            jmp.label = tac_ins.target.var;
            push_vectorASM_Ins(v, jmp);
            break;
        }
        case TAC_INS_JMP_IF_ZERO:
            assemble_jmp(false, tac_ins, v);
            break;
        case TAC_INS_JMP_IF_NOT_ZERO:
            assemble_jmp(true, tac_ins, v);
            break;
        case TAC_INS_BINARY_EQUAL:
            assemble_comp(ASM_COND_E, tac_ins, v);
            break;
        case TAC_INS_BINARY_NOT_EQUAL:
            assemble_comp(ASM_COND_NE, tac_ins, v);
            break;
        case TAC_INS_BINARY_GREATER_THAN:
            assemble_comp(ASM_COND_G, tac_ins, v);
            break;
        case TAC_INS_BINARY_LESS_THAN:
            assemble_comp(ASM_COND_L, tac_ins, v);
            break;
        case TAC_INS_BINARY_GREATER_OR_EQUAL:
            assemble_comp(ASM_COND_GE, tac_ins, v);
            break;
        case TAC_INS_BINARY_LESS_OR_EQUAL:
            assemble_comp(ASM_COND_LE, tac_ins, v);
            break;
        case TAC_INS_COPY:
            assemble_mov(assemble_operand(tac_ins.src), 
                         assemble_operand(tac_ins.unary_dst), v);
            break;
        case TAC_INS_LABEL:
        {
            ASM_Ins label;
            label.type = ASM_INS_LABEL;
            label.label = tac_ins.label.var;
            push_vectorASM_Ins(v, label);
            break;
        }
    }
}

ASM_Function assemble_function(TAC_Function function) {
    ASM_Function ret;
    ret.name = function.name;
    init_vectorASM_Ins(&ret.instructions, 2);
    ASM_Ins a;
    a.type = ASM_INS_ALLOCATE;
    a.alloc = function.var_cnt;
    push_vectorASM_Ins(&ret.instructions, a);
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
