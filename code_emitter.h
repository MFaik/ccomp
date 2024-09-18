#pragma once

#include <stdio.h>

#include "asm_generator.h"

typedef enum {
    LINUX,
    MAC,
} OS;

void write_program(ASM_Program program, FILE* _file, OS _os);
