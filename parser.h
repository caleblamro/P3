#ifndef __PARSER__H__
#define __PARSER__H__

#include <string>
#include <vector>
#include "execute.h"
#include "lexer.h"
#include <map>
#include <vector>


struct Program {
    std::map<int, int> mem;
    std::map<std::string, int> mem_mappings;
    InstructionNode* start;
    std::vector<int> inputs;
};
struct Expression {
    int op1_location;
    ArithmeticOperatorType op_type;
    int op2_location;
};

struct Condition {
    int op1_location;
    ConditionalOperatorType type;
    int op2_location;
};

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
