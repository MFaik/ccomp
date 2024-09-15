#pragma once
#include "vector.h"
#include "string_view.h"
#include "parser.h"

typedef enum{
    OP_IMM,
    OP_REG,
} OpType;
typedef struct {
    OpType type;
    int imm;
} ASM_Operand;

typedef struct {
    ASM_Operand src;
    ASM_Operand dst;
} ASM_Mov;

typedef enum{
    INS_MOV,
    INS_RET,
} InsType;
typedef struct {
    InsType type;
    ASM_Mov mov;
} ASM_Ins;

vector_header(ASM_Ins);
typedef struct {
    StringView name;
    VectorASM_Ins instructions;
} ASM_Function;

typedef struct {
    ASM_Function function;
} ASM_Program;

ASM_Program assemble_program(AST_Program program);
