#include "lexer.h"
#include "parser.h"


struct InstructionNode * parse_Generate_Intermediate_Representation()
{
    Parser p;
    Program program = p.parse_program();
    
    inputs = program.inputs;

    for(auto& pair : program.mem) {
        mem[pair.first] = pair.second;
    }

    return program.start;
}