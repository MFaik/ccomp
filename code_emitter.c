#include "code_emitter.h"

#include <stdio.h>

#include "asm_generator.h"
#include "string_view.h"

static OS os;
static FILE* file;

#define fprintf_space(...) fprintf(file, "    ");fprintf(__VA_ARGS__);

void write_operand(ASM_Operand operand) {
    switch(operand.type) {
        case OP_IMM:
            fprintf(file, "$%d", operand.imm);
            break;
        case OP_REG_AX:
            fprintf(file, "%%eax");
            break;
        case OP_REG_R10:
            fprintf(file, "%%r10d");
            break;
        case OP_PSEUDO:
            fprintf(file, "%d(%%rbp)", (operand.pseudo+1)*-4);
            break;
    }
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
        case ASM_INS_MOV:
            fprintf_space(file, "movl ");
            write_operand(instruction.src);
            fprintf(file, ", ");
            write_operand(instruction.dst);
            fprintf(file, "\n");
            break;
        case ASM_INS_UNARY_NEG:
            fprintf_space(file, "negl ");
            write_operand(instruction.op);
            fprintf(file, "\n");
            break;
        case ASM_INS_UNARY_COMPLEMENT:
            fprintf_space(file, "notl ");
            write_operand(instruction.op);
            fprintf(file, "\n");
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
