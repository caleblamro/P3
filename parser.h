#ifndef __PARSER__H__
#define __PARSER__H__
/**
 * Author: Caleb Lamoreaux
 * Project: Project 3 CSE 340 Fall 2023
 * ASU_ID: 1222071513
 * Description: Header file for the Parser class. Contains other structs used in parser.cc as well.
*/

#include <string>
#include <vector>
#include "execute.h"
#include "lexer.h"
#include <map>
#include <vector>

// Used to contain the information of the program during compile time. Also used in parse_Generate_Intermediate_Representation.
struct Program {
    std::map<int, int> mem;
    std::map<std::string, int> mem_mappings; // Used to associate Token.lexeme with a memory location
    InstructionNode* start;
    std::vector<int> inputs;
};
// A simple struct for an expression used with assign_stmts
struct Expression {
    int op1_location;
    ArithmeticOperatorType op_type;
    int op2_location;
};
// Similar to expression, used to contain the information needed for a condition
struct Condition {
    int op1_location;
    ConditionalOperatorType type;
    int op2_location;
};
// This is a vital part of the program, as it allows O(N) append operations to the instructions
struct InstructionList {
    InstructionNode* start;
    InstructionNode* end;
};


class Parser {
  public:
        Parser();
        Program parse_program();
        void parse_var_section();
        void parse_id_list();
        InstructionList parse_body();
        InstructionList parse_stmt_list();
        InstructionList parse_stmt();
        InstructionList parse_assign_stmt();
        Expression parse_expr();
        int parse_primary();
        ArithmeticOperatorType parse_op();
        InstructionList parse_output_stmt();
        InstructionList parse_input_stmt();
        InstructionList parse_while_stmt();
        InstructionList parse_if_stmt();
        Condition parse_condition();
        ConditionalOperatorType parse_relop();
        InstructionList parse_switch_stmt();
        InstructionList parse_for_stmt();
        void parse_case_list(int, InstructionNode*, std::vector<InstructionList>&);
        InstructionList parse_case(int, InstructionNode*);
        InstructionList parse_default_case(InstructionNode*);
        void parse_inputs();
        void parse_num_list();
        int parse_constant(Token);
  private:
        LexicalAnalyzer lexer;
        Program program;
        int variable_counter;
        Token expect(TokenType);
        int location(std::string);
};

#endif  //__PARSER__H__
