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
            TAC_Val dst;
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
