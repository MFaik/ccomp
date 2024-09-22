#include <stdio.h>

#include "read_file.h"
#include "lexer.h"
#include "parser.h"
#include "tacky.h"
#include "asm_generator.h"
#include "code_emitter.h"

int main(int argc, char **argv) {
    if(argc < 3)
        return 1;
    //read file
    char* code = read_file(argv[2]);
    //lexer
    VectorTerm terms = lex(code);
    if(!terms.size) {
        return 1;
    }
    if(argv[1][0] == 'l') {
        for(int i = 0;i < terms.size;i++) {
            printf("%d ", i);
            pretty_print_term(terms.array[i]);
            printf("\n");
        }
        return 0;
    }
    //parser
    AST_Program ast_program = parse_program(terms);
    if(ast_program.error)
        return ast_program.error;
    if(argv[1][0] == 'p') {
        pretty_print_program(ast_program);
        return 0;
    }
    //tacky
    TAC_Program tac_program = emit_tacky(ast_program);
    if(argv[1][0] == 't') {
        pretty_print_tacky_program(tac_program);
        return 0;
    }
    //assembler
    ASM_Program asm_program = assemble_program(tac_program);
    //code_emitter
    FILE* file;
    if(argc == 3) {
        file = fopen("a.s", "w");
    } else {
        file = fopen(argv[argc-1], "w");
    }
    
    write_program(asm_program, file, LINUX);
    fclose(file);

    //TODO: clean function instructions
    free_vectorTerm(&terms);
    free(code);
    return 0;
}
