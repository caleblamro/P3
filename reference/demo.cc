/*
Abel Haddis and I worked on this together before the solo project rule, Got permission to submit together
We worked on the code together
 */
#include "Parser.h"
#include "compiler.h"
#include <map>
#include <iostream>

using namespace std;

InstructionNode* parse_generate_intermediate_representation() {
    Parser parser;
    ParseResult result = parser.parse_program();
    inputs = result.inputs;

    for (map<int, int>::iterator iter = result.StartingMemory.begin(); iter != result.StartingMemory.end(); ++iter) {
        mem[iter->first] = iter->second;
    }

    return result.program;

}