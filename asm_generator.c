#include "asm_generator.h"

#include <stdbool.h>

#include "tacky.h"
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
    ASM_Operand dst = assemble_operand(tac_ins.unary_dst);

    assemble_mov(src, dst, v);

    ASM_Ins ins;
    ins.type = type;
    ins.op = dst;
    insert_vectorASM_Ins(v, ins);
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
        insert_vectorASM_Ins(v, ins);
        assemble_mov((ASM_Operand){OP_REG_R11}, dst, v);
    } else if(type == ASM_INS_BINARY_LEFT_SHIFT || type == ASM_INS_BINARY_RIGHT_SHIFT) {
        assemble_mov(src1, dst, v);
        ins.dst = dst;
        assemble_mov(src2, (ASM_Operand){OP_REG_CX}, v);
        ins.src = (ASM_Operand){OP_REG_CX};
        insert_vectorASM_Ins(v, ins);
    } else {
        assemble_mov(src1, dst, v);
        ins.dst = dst;
        if(src2.type == OP_PSEUDO && dst.type == OP_PSEUDO) {
            assemble_mov(src2, (ASM_Operand){OP_REG_R10}, v);
            ins.src = (ASM_Operand){OP_REG_R10};
        } else {
            ins.src = src2;
        }
        insert_vectorASM_Ins(v, ins);
    }
}

void assemble_div(bool is_remainder, TAC_Ins tac_ins, VectorASM_Ins *v) {
    ASM_Operand src1 = assemble_operand(tac_ins.src1);
    ASM_Operand src2 = assemble_operand(tac_ins.src2);
    ASM_Operand dst = assemble_operand(tac_ins.unary_dst);
    assemble_mov(src1, (ASM_Operand){OP_REG_AX}, v);
    insert_vectorASM_Ins(v, (ASM_Ins){ASM_INS_CDQ});
    ASM_Ins ins;
    ins.type = ASM_INS_UNARY_IDIV;
    if(src2.type == OP_IMM) {
        assemble_mov(src2, (ASM_Operand){OP_REG_R10}, v);
        ins.op = (ASM_Operand){OP_REG_R10};
    } else {
        ins.op = src2;
    }
    insert_vectorASM_Ins(v, ins);
    if(is_remainder) {
        assemble_mov((ASM_Operand){OP_REG_DX}, dst, v);
    } else {
        assemble_mov((ASM_Operand){OP_REG_AX}, dst, v);
    }
}

void assemble_instruction(TAC_Ins tac_ins, VectorASM_Ins *v) {
    ASM_Ins asm_ins;
    switch(tac_ins.type) {
        case TAC_INS_RETURN:
            {
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
            jmp.label = tac_ins.label.var;
            insert_vectorASM_Ins(v, jmp);
            break;
        }
        case TAC_INS_JMP_IF_ZERO:
            assemble_jump();
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
