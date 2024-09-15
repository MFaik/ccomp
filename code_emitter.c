#include <stdio.h>
#include "read_file.h"
#include "lexer.h"
#include "parser.h"
#include "asm_generator.h"
#include "string_view.h"

typedef enum {
    LINUX,
    MAC,
} OS;

OS os;
FILE* file;

void write_operand(ASM_Operand operand) {
    switch(operand.type) {
        case OP_IMM:
            fprintf(file, "$%d", operand.imm);
            break;
        case OP_REG:
            fprintf(file, "%%eax");
            break;
    }
}

void write_instruction(ASM_Ins instruction) {
    switch(instruction.type) {
        case INS_RET:
            fprintf(file, "    ret\n");
            break;
        case INS_MOV:
            fprintf(file, "    movl ");
            write_operand(instruction.mov.src);
            fprintf(file, ", ");
            write_operand(instruction.mov.dst);
            fprintf(file, "\n");
            break;
    }
}

void write_function(ASM_Function function) {
    if(os == MAC) {
        fprintf(file, "    .globl _%.*s\n", function.name.len, function.name.start);
        fprintf(file, "_%.*s:\n", function.name.len, function.name.start);
    } else if(os == LINUX) {
        fprintf(file, "    .globl %.*s\n", function.name.len, function.name.start);
        fprintf(file, "%.*s:\n", function.name.len, function.name.start);
    }
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

int main(int argc, char **argv) {
    if(argc != 2)
        return 1;
    char* code = read_file(argv[1]);
    VectorTerm terms = lex(code);
    AST_Program ast_program = parse_program(terms);
    if(ast_program.error)
        return ast_program.error;
    ASM_Program  asm_program = assemble_program(ast_program);

    FILE* file = fopen("a.s", "w");
    write_program(asm_program, file, LINUX);
    fclose(file);

    //TODO: clean function instructions
    free_vectorTerm(&terms);
    free(code);
    return 0;
}
