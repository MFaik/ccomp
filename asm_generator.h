#pragma once

#include "vector.h"
#include "string_view.h"
#include "tacky.h"

typedef enum{
    OP_IMM,
    OP_REG_AX,
    OP_REG_DX,
    OP_REG_CX,
    OP_REG_R10,
    OP_REG_R11,
    OP_PSEUDO,
} OperandType;
typedef struct {
    OperandType type;
    union {
        int imm;
        unsigned pseudo;
    };
} ASM_Operand;

typedef enum{
    ASM_COND_E,
    ASM_COND_NE,
    ASM_COND_G,
    ASM_COND_GE,
    ASM_COND_L,
    ASM_COND_LE,
} ASM_Cond;
typedef enum{
    ASM_INS_RET,
    ASM_INS_MOV,
    ASM_INS_CDQ,
    ASM_INS_UNARY_NEG,
    ASM_INS_UNARY_COMPLEMENT,
    ASM_INS_UNARY_IDIV,
    ASM_INS_BINARY_ADD,
    ASM_INS_BINARY_SUB,
    ASM_INS_BINARY_MUL,
    ASM_INS_BINARY_BITWISE_AND,
    ASM_INS_BINARY_BITWISE_OR,
    ASM_INS_BINARY_BITWISE_XOR,
    ASM_INS_BINARY_LEFT_SHIFT,
    ASM_INS_BINARY_RIGHT_SHIFT,
    ASM_INS_ALLOCATE,
    ASM_INS_CMP,
    ASM_INS_JMP,
    ASM_INS_JMPCC,
    ASM_INS_SETCC,
    ASM_INS_LABEL,
} ASM_InsType;
typedef struct {
    ASM_InsType type;
    union {
        struct {
            ASM_Operand src;
            ASM_Operand dst;
        };
        struct {
            ASM_Cond cond;
            unsigned label;
        };
        struct {
            ASM_Cond PADDING;
            ASM_Operand set_op;
        };
        ASM_Operand op;
        unsigned alloc;
    };
} ASM_Ins;

vector_header(ASM_Ins);
typedef struct {
    StringView name;
    VectorASM_Ins instructions;
} ASM_Function;

typedef struct {
    ASM_Function function;
} ASM_Program;

ASM_Program assemble_program(TAC_Program program);
