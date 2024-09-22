#pragma once

#include "string_view.h"
#include "vector.h"
#include "parser.h"

typedef enum {
    TAC_VAL_CONSTANT,
    TAC_VAL_VAR
} TAC_Val_Type;
typedef enum {
    TAC_INS_RETURN,
    TAC_INS_UNARY_NEG,
    TAC_INS_UNARY_COMPLEMENT,
    TAC_INS_BINARY_ADD,
    TAC_INS_BINARY_SUB,
    TAC_INS_BINARY_MUL,
    TAC_INS_BINARY_DIV,
    TAC_INS_BINARY_REMAINDER,
    TAC_INS_BINARY_BITWISE_AND,
    TAC_INS_BINARY_BITWISE_OR,
    TAC_INS_BINARY_BITWISE_XOR,
    TAC_INS_BINARY_LEFT_SHIFT,
    TAC_INS_BINARY_RIGHT_SHIFT,
} TAC_Ins_Type;

typedef struct {
    TAC_Val_Type type;
    union {
        long constant;
        unsigned var;
    };
} TAC_Val;

typedef struct {
    TAC_Ins_Type type;
    union {
        TAC_Val ret;
        struct {
            TAC_Val src;
            //dst must be var
            TAC_Val unary_dst;
        };
        struct {
            TAC_Val src1;
            TAC_Val src2;
            //dst must be var
            TAC_Val binary_dst;
        };
    };
} TAC_Ins;
vector_header(TAC_Ins)
typedef struct {
    StringView name;
    VectorTAC_Ins instructions;
    unsigned var_cnt;
} TAC_Function;

typedef struct {
    TAC_Function function;
} TAC_Program;

TAC_Program emit_tacky(AST_Program ast_program);
void pretty_print_tacky_program(TAC_Program tac_program);
