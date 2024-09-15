#include "asm_generator.h"

#include "vector.h"

vector_body(ASM_Ins);

ASM_Operand assemble_imm(int constant) {
    return (ASM_Operand){OP_IMM, constant};
}

ASM_Operand assmeble_register() {
    return (ASM_Operand){OP_REG};
}

void assemble_statement(AST_Statement statement, VectorASM_Ins *v) {
    insert_vectorASM_Ins(v,
            (ASM_Ins){INS_MOV,
                (ASM_Mov){assemble_imm(statement.ret.constant),
                          assmeble_register()}});
    insert_vectorASM_Ins(v, (ASM_Ins){INS_RET});
}

ASM_Function assemble_function(AST_Function function) {
    ASM_Function ret;
    ret.name = function.name.str;
    init_vectorASM_Ins(&ret.instructions, 2);
    assemble_statement(function.statement, &ret.instructions);
    return ret;
}

ASM_Program assemble_program(AST_Program program) {
    ASM_Program ret;
    ret.function = assemble_function(program.function);
    return ret;
}

