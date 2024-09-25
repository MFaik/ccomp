#include "code_emitter.h"

#include <stdio.h>

#include "asm_generator.h"
#include "string_view.h"

static OS os;
static FILE* file;

#define fprintf_space(...) fprintf(file, "    ");fprintf(__VA_ARGS__);

void write_operand(ASM_Operand operand, char byte) {
    switch(operand.type) {
        case OP_IMM:
            fprintf(file, "$%d", operand.imm);
            break;
        case OP_REG_AX:
            if(byte == 4)
                fprintf(file, "%%eax");
            else if(byte == 1)
                fprintf(file, "%%al");
            break;
        case OP_REG_CX:
            if(byte == 4)
                fprintf(file, "%%ecx");
            else if(byte == 1)
                fprintf(file, "%%cl");
            break;
        case OP_REG_DX:
            if(byte == 4)
                fprintf(file, "%%edx");
            else if(byte == 1)
                fprintf(file, "%%dl");
            break;
        case OP_REG_R10:
            if(byte == 4)
                fprintf(file, "%%r10d");
            else if(byte == 1)
                fprintf(file, "%%r10b");
            break;
        case OP_REG_R11:
            if(byte == 4)
                fprintf(file, "%%r11d");
            else if(byte == 1)
                fprintf(file, "%%r11b");
            break;
        case OP_PSEUDO:
            fprintf(file, "%d(%%rbp)", (operand.pseudo+1)*-4);
            break;
    }
}

void write_cond(ASM_Cond cond) {
    switch(cond) {
        case ASM_COND_E:
            fprintf(file, "e");
            break;
        case ASM_COND_NE:
            fprintf(file, "ne");
            break;
        case ASM_COND_G:
            fprintf(file, "g");
            break;
        case ASM_COND_L:
            fprintf(file, "l");
            break;
        case ASM_COND_GE:
            fprintf(file, "ge");
            break;
        case ASM_COND_LE:
            fprintf(file, "le");
            break;
    }
}

void write_label(unsigned label) {
    if(os == LINUX)
        fprintf(file, ".");
    fprintf(file, "L%u", label);
}

void write_instruction_binary(const char* ins, ASM_Ins instruction, char byte) {
    fprintf_space(file, "%s ", ins);
    write_operand(instruction.src, byte);
    fprintf(file, ", ");
    write_operand(instruction.dst, byte);
    fprintf(file, "\n");
}

void write_instruction(ASM_Ins instruction) {
    switch(instruction.type) {
        case ASM_INS_ALLOCATE:
            fprintf_space(file, "subq $%d, %%rsp\n", instruction.alloc*4);
            break;
        case ASM_INS_RET:
            fprintf_space(file, "movq %%rbp, %%rsp\n");
            fprintf_space(file, "popq %%rbp\n");
            fprintf_space(file, "ret\n");
            break;
        case ASM_INS_CDQ:
            fprintf_space(file, "cdq\n");
            break;
        case ASM_INS_UNARY_NEG:
            fprintf_space(file, "negl ");
            write_operand(instruction.op, 4);
            fprintf(file, "\n");
            break;
        case ASM_INS_UNARY_COMPLEMENT:
            fprintf_space(file, "notl ");
            write_operand(instruction.op, 4);
            fprintf(file, "\n");
            break;
        case ASM_INS_UNARY_IDIV:
            fprintf_space(file, "idiv ");
            write_operand(instruction.op, 4);
            fprintf(file, "\n");
            break;
        case ASM_INS_MOV:
            write_instruction_binary("movl", instruction, 4);
            break;
        case ASM_INS_BINARY_ADD:
            write_instruction_binary("addl", instruction, 4);
            break;
        case ASM_INS_BINARY_SUB:
            write_instruction_binary("subl", instruction, 4);
            break;
        case ASM_INS_BINARY_BITWISE_AND:
            write_instruction_binary("andl", instruction, 4);
            break;
        case ASM_INS_BINARY_BITWISE_OR:
            write_instruction_binary("orl", instruction, 4);
            break;
        case ASM_INS_BINARY_BITWISE_XOR:
            write_instruction_binary("xorl", instruction, 4);
            break;
        case ASM_INS_BINARY_LEFT_SHIFT:
            write_instruction_binary("shll", instruction, 4);
            break;
        case ASM_INS_BINARY_RIGHT_SHIFT:
            write_instruction_binary("shrl", instruction, 4);
            break;
        case ASM_INS_BINARY_MUL:
            write_instruction_binary("imull", instruction, 4);
            break;
        case ASM_INS_CMP:
            write_instruction_binary("cmpl", instruction, 4);
            break;
        case ASM_INS_JMP:
            fprintf_space(file, "jmp ");
            write_label(instruction.label);
            fprintf(file, "\n");
            break;
        case ASM_INS_JMPCC:
            fprintf_space(file, "j");
            write_cond(instruction.cond);
            fprintf(file, " ");
            write_label(instruction.label);
            fprintf(file, "\n");
            break;
        case ASM_INS_SETCC:
            fprintf_space(file, "set");
            write_cond(instruction.cond);
            fprintf(file, " ");
            write_operand(instruction.set_op, 1);
            fprintf(file, "\n");
            break;
        case ASM_INS_LABEL:
            write_label(instruction.label);
            fprintf(file, ":\n");
            break;
    }
}

void write_function(ASM_Function function) {
    if(os == MAC) {
        fprintf_space(file, ".globl _%.*s\n", function.name.len, function.name.start);
        fprintf(file, "_%.*s:\n", function.name.len, function.name.start);
    } else if(os == LINUX) {
        fprintf_space(file, ".globl %.*s\n", function.name.len, function.name.start);
        fprintf(file, "%.*s:\n", function.name.len, function.name.start);
    }
    fprintf_space(file, "pushq %%rbp\n");
    fprintf_space(file, "movq %%rsp, %%rbp\n");
    for(int i = 0;i < function.instructions.size;i++) {
        write_instruction(function.instructions.array[i]);
    }
}

void write_program(ASM_Program program, FILE* _file, OS _os) {
    file = _file;
    os = _os;
    write_function(program.function);
    if(os == LINUX)
        fprintf(file, "\n.section .note.GNU-stack,\"\",@progbits\n");
}
