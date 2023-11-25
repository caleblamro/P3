/**
 * Author: Caleb Lamoreaux
 * Project: Project 3 CSE 340 Fall 2023
 * ASU_ID: 1222071513
 * Description: Main file for Project 3, parses and constructs the program to be executed. Essentially a very simple compiler.
*/
#include "parser.h"
#include <iostream>

// Although it will not be needed, this will be used in spots that would cause an error, since there are no errors, it will never be called
void syntax_error() {
    std::cout << "SYNTAX ERROR!!!" << std::endl;
    exit(0);
}

// Checks if an ID has been added to the mem_mappings
int Parser::location(std::string id) {
    auto it = program.mem_mappings.find(id);
    if(it == program.mem_mappings.end()) {
        return -1;
    }else{
        return program.mem_mappings[id];
    }
}
// Construct a no-op node
InstructionNode* get_no_operation() {
    InstructionNode* stmt = new InstructionNode();
    stmt->type = InstructionType::NOOP;
    stmt->next = NULL;
    return stmt;
}

// This expect consumes the token. Since there should be no syntax errors, this works just fine
Token Parser::expect(TokenType t) {
    Token tkn = lexer.GetToken();
    if(tkn.token_type != t) {
        syntax_error();
    }
    return tkn;
}

Parser::Parser() {
    variable_counter = 0;
}

// Used to parse NUM and only NUM, adds the constant to mem_mappings and mem if not present already
int Parser::parse_constant(Token t) {
    int result = location(t.lexeme);

    if(result == -1) {
        program.mem_mappings[t.lexeme] = variable_counter;
        // Assign the constant value to the memory location
        program.mem[variable_counter++] = std::stoi(t.lexeme);
    }
    return location(t.lexeme);
}
// Entry point for parsing
Program Parser::parse_program() {
    // Consume the var_section
    parse_var_section();
    // At this point, we've populated program.mem_mappings with all the variable names declared in var_section
    InstructionList body = parse_body();
    program.start = body.start;

    // Parse and populate inputs
    parse_inputs();

    return program;
}

// Method to parse the var section
void Parser::parse_var_section() {
    parse_id_list();
    expect(TokenType::SEMICOLON);
}

void Parser::parse_id_list() {
    Token t = expect(TokenType::ID);
    // No need to check if we've inserted a name already, instructions say to assume all names are unique
    program.mem_mappings[t.lexeme] = variable_counter;
    //Initialize this variable to zero
    program.mem[variable_counter++] = 0;
    // We may see a comma here, which tells us to parse another id list
    Token maybe_comma = lexer.peek(1);
    if(maybe_comma.token_type == TokenType::COMMA) {
        lexer.GetToken();
        parse_id_list();
    }
}
// Method to parse the body
InstructionList Parser::parse_body() {
    expect(TokenType::LBRACE);

    InstructionList list = parse_stmt_list();

    expect(TokenType::RBRACE);
    return list;
}

InstructionList Parser::parse_stmt_list() {
    InstructionList parsed_stmt = parse_stmt();
    Token t = lexer.peek(1);
    // We know this is the last stmt if after the stmt we see an RBRACE, which closes out the stmt_list
    if(t.token_type == TokenType::RBRACE) {
        return parsed_stmt;
    }
    // This is quite verbose, but it simply checks if t.token_type is in FIRST(stmt)
    if(t.token_type == TokenType::ID || t.token_type == TokenType::WHILE || t.token_type == TokenType::IF|| t.token_type == TokenType::SWITCH 
    || t.token_type == TokenType::FOR || t.token_type == TokenType::OUTPUT || t.token_type == TokenType::INPUT) {
        // At this point, we know we have another statement to parse, so we can call parse_stmt_list recursively
        // We also need to link the statements together, so we will set the end of the previous parsed_stmt to the start of the current parsed_list
        InstructionList parsed_list = parse_stmt_list();
        parsed_stmt.end->next = parsed_list.start;

        // Construct the InstructionList
        InstructionList result;
        result.start = parsed_stmt.start;
        result.end = parsed_list.end;
        return result;
    }
    syntax_error();
}
// Method to parse a statement
InstructionList Parser::parse_stmt() {
    Token t = lexer.peek(1);
    // This simply checks the token t for a terminal that is FIRST(stmt_type)
    if(t.token_type == TokenType::ID) {
        return parse_assign_stmt();
    }else if(t.token_type == TokenType::WHILE) {
        return parse_while_stmt();
    }else if(t.token_type == TokenType::IF) {
        return parse_if_stmt();
    }else if(t.token_type == TokenType::SWITCH) {
        return parse_switch_stmt();
    }else if(t.token_type == TokenType::FOR){
        return parse_for_stmt();
    }else if(t.token_type == TokenType::OUTPUT) {
        return parse_output_stmt();
    }else if(t.token_type == TokenType::INPUT) {
        return parse_input_stmt();
    }else{
        // We don't really need this, since there aren't any syntax or semantic errors in this project, but I'll add for my own sake
        syntax_error();
    }
}
// Parses the assignment statement and returns an instruction list with the assign statement as the only instruction in the list
InstructionList Parser::parse_assign_stmt() {
    // Create empty assign stmt
    InstructionNode* stmt = new InstructionNode();
    stmt->type = InstructionType::ASSIGN;
    stmt->next = NULL;

    // Extract LHS variable info
    Token var_id = expect(TokenType::ID);
    int var_location = location(var_id.lexeme);
    // Assign the location to the index of the LHS
    stmt->assign_inst.left_hand_side_index = var_location;

    expect(TokenType::EQUAL);

    // Here we peek(2) since it's the only way to differentiate between and expr and primary
    // The token will be a semicolon if its primary and otherwise it must be an expression
    Token op = lexer.peek(2);
    if(op.token_type == TokenType::SEMICOLON) {
        int primary_location = parse_primary();
        stmt->assign_inst.opernd1_index = primary_location;
        stmt->assign_inst.op = ArithmeticOperatorType::OPERATOR_NONE;
    }else{
        Expression e = parse_expr();
        stmt->assign_inst.opernd1_index = e.op1_location;
        stmt->assign_inst.opernd2_index = e.op2_location;
        stmt->assign_inst.op = e.op_type;
    }
    expect(TokenType::SEMICOLON);
    // Make an InstructionList with one node (need to set start and end to the stmt since it is the first and last node in the list)
    InstructionList result;
    result.start = stmt;
    result.end = stmt;
    return result;
}

// Parses the expression and returns all the required information for the instruction
Expression Parser::parse_expr() {
    int op1_location = parse_primary();
    ArithmeticOperatorType op_type = parse_op();
    int op2_location = parse_primary();
    Expression e;
    e.op1_location = op1_location;
    e.op_type = op_type;
    e.op2_location = op2_location;
    return e;
}

// Parse a primary token, and return its location in memory
int Parser::parse_primary() {
    Token t = lexer.GetToken();
    if(t.token_type == TokenType::ID) {
        // If it's an ID, return the location of it
        return location(t.lexeme);
    }else if(t.token_type == TokenType::NUM) {
        // If it's a constant, parse it, and return its location
        return parse_constant(t);
    }
}

// Simply parses an operator and returns its type
ArithmeticOperatorType Parser::parse_op() {
    Token t = lexer.GetToken();
    if(t.token_type == TokenType::PLUS) {
        return ArithmeticOperatorType::OPERATOR_PLUS;
    }else if(t.token_type == TokenType::MINUS) {
        return ArithmeticOperatorType::OPERATOR_MINUS;
    }else if(t.token_type == TokenType::MULT) {
        return ArithmeticOperatorType::OPERATOR_MULT;
    }else if(t.token_type == TokenType::DIV) {
        return ArithmeticOperatorType::OPERATOR_DIV;
    }
    // Again, not needed, but I'll add to make myself feel better
    syntax_error();
}

// Parses an ouput statement and returns an instruction list containing it
InstructionList Parser::parse_output_stmt() {
    InstructionNode* stmt = new InstructionNode();
    stmt->type = InstructionType::OUT;
    stmt->next = NULL;

    expect(TokenType::OUTPUT);
    Token t = expect(TokenType::ID);
    expect(TokenType::SEMICOLON);

    stmt->output_inst.var_index = location(t.lexeme);

    InstructionList result;
    result.start = stmt;
    result.end = stmt;
    return result;
}

// Similar to parse_output_stmt, this parses the stmt and returns it as an InstructionList
InstructionList Parser::parse_input_stmt() {
    InstructionNode* stmt = new InstructionNode();
    stmt->type = InstructionType::IN;
    stmt->next = NULL;

    expect(TokenType::INPUT);
    Token t = expect(TokenType::ID);
    expect(TokenType::SEMICOLON);

    stmt->input_inst.var_index = location(t.lexeme);

    InstructionList result;
    result.start = stmt;
    result.end = stmt;
    return result;
}
// Parses and returns the while statement
InstructionList Parser::parse_while_stmt() {
    // Create the CJMP node
    InstructionNode* stmt = new InstructionNode();
    stmt->type = InstructionType::CJMP;
    stmt->next = NULL;
    
    expect(TokenType::WHILE);
    
    Condition condition = parse_condition();
    
    // Set up the conditional jump based on the results of the condition
    stmt->cjmp_inst.condition_op = condition.type;
    stmt->cjmp_inst.opernd1_index = condition.op1_location;
    stmt->cjmp_inst.opernd2_index = condition.op2_location;
    InstructionNode* no_operation = get_no_operation();
    // Jump to after the while loop if CJMP is false
    stmt->cjmp_inst.target = no_operation;


    // Create the jump node which jumps back to the conditional jump
    InstructionNode* jump = new InstructionNode();
    jump->type = InstructionType::JMP;
    jump->jmp_inst.target = stmt;


    InstructionList body = parse_body();
    // If the stmt is true, go to body
    stmt->next = body.start;
    // Jump back to the CJMP node to continue the loop
    body.end->next = jump;


    InstructionList result;
    result.start = stmt;
    result.end = no_operation;
    return result;
}

InstructionList Parser::parse_if_stmt() {
    // Set up if stmt node
    InstructionNode* stmt = new InstructionNode();
    stmt->type = InstructionType::CJMP;
    stmt->next = NULL;
    
    // Consume the IF token
    expect(TokenType::IF);
    
    Condition condition = parse_condition();
    
    // Set up the conditional jump based on the results of the condition
    stmt->cjmp_inst.condition_op = condition.type;
    stmt->cjmp_inst.opernd1_index = condition.op1_location;
    stmt->cjmp_inst.opernd2_index = condition.op2_location;

    // Get a no_operation, which will be appended to the resulting parsed body
    InstructionNode* no_operation = get_no_operation();
    stmt->cjmp_inst.target = no_operation;
    
    
    InstructionList body = parse_body();
    stmt->next = body.start;
    body.end->next = no_operation;


    InstructionList result;
    result.start = stmt;
    result.end = no_operation;
    return result;
}

Condition Parser::parse_condition() {
    Condition c;
    
    int op1_location = parse_primary();
    ConditionalOperatorType op_type = parse_relop();
    int op2_location = parse_primary();

    c.op1_location = op1_location;
    c.type = op_type;
    c.op2_location = op2_location;

    return c;
}
// Simply parses the relop and returns its type
ConditionalOperatorType Parser::parse_relop() {
    Token t = lexer.GetToken();
    if(t.token_type == TokenType::GREATER) {
        return ConditionalOperatorType::CONDITION_GREATER;
    }else if(t.token_type == TokenType::LESS) {
        return ConditionalOperatorType::CONDITION_LESS;
    }else if(t.token_type == TokenType::NOTEQUAL) {
        return ConditionalOperatorType::CONDITION_NOTEQUAL;
    }
    syntax_error();
}
// Parses, constructs, and returns a switch statement
InstructionList Parser::parse_switch_stmt() {
    expect(TokenType::SWITCH);
    
    // Get location of variable to switch on
    int var_location = location(expect(TokenType::ID).lexeme);

    expect(TokenType::LBRACE);
    
    // Set up a location for after the switch statement
    InstructionNode* end_of_switch = get_no_operation();
    
    std::vector<InstructionList> cases;
    parse_case_list(var_location, end_of_switch, cases);

    Token t = lexer.peek(1);
    // If we have a default case, add it to the InstructionList of cases
    if(t.token_type == TokenType::DEFAULT) {
        // Parses the default case and sets the instruction after the InstructionList to the end_of_switch
        InstructionList default_case = parse_default_case(end_of_switch);
        // This chains the cases together in such a way that each case is checked one after another
        // Jumping to the next case if false, and continuing to the body if true
        for(size_t i = 0; i < cases.size() - 1; i++) {
            cases[i].start->next = cases[i+1].start;
        }
        cases[cases.size() - 1].start->next = default_case.start;
    }else{
        // Same as above except there's no default case being added
        for(size_t i = 0; i < cases.size() - 1; i++) {
            cases[i].start->next = cases[i+1].start;
        }
        cases[cases.size() - 1].start->next = end_of_switch;
    }

    expect(TokenType::RBRACE);

    InstructionList result;
    result.start = cases[0].start;
    result.end = end_of_switch;
    return result;
}
// Parses, constructs, and returns a for statement
InstructionList Parser::parse_for_stmt() {
    expect(TokenType::FOR);
    expect(TokenType::LPAREN);
    // Parse the assignment statement as a single instruction (if you look at the function all it does it set the start and end to the same InstructionNode*)
    InstructionNode* assign_stmt = parse_assign_stmt().start;
    InstructionNode* end_of_for = get_no_operation();

    Condition condition = parse_condition();

    InstructionNode* condition_stmt = new InstructionNode();
    condition_stmt->type = InstructionType::CJMP;
    condition_stmt->cjmp_inst.target = end_of_for;
    condition_stmt->cjmp_inst.opernd1_index = condition.op1_location;
    condition_stmt->cjmp_inst.opernd2_index = condition.op2_location;
    condition_stmt->cjmp_inst.condition_op = condition.type;

    expect(TokenType::SEMICOLON);
    // This is the assignment statement we will be doing after the body every iteration
    InstructionNode* assign_stmt_repeat = parse_assign_stmt().start;
    expect(TokenType::RPAREN);

    InstructionList body = parse_body();

    InstructionNode* jump = new InstructionNode();
    jump->type = InstructionType::JMP;
    jump->jmp_inst.target = condition_stmt;

    // Tie together the statement
    assign_stmt->next = condition_stmt;
    condition_stmt->next = body.start;
    body.end->next = assign_stmt_repeat;
    assign_stmt_repeat->next = jump;

    InstructionList result;
    result.start = assign_stmt;
    result.end = end_of_for;
    return result;
}
// Parses the cases in a switch statement and adds them to list
void Parser::parse_case_list(int var_location, InstructionNode* end_of_switch, std::vector<InstructionList>& list) {
    InstructionList parsed_case = parse_case(var_location, end_of_switch);
    list.push_back(parsed_case);
    Token t = lexer.peek(1);
    if(t.token_type == TokenType::CASE) {
        parse_case_list(var_location, end_of_switch, list);
    }
}
// Parses and returns a case
InstructionList Parser::parse_case(int var_location, InstructionNode* end_of_switch) {
    InstructionNode* equality_check = new InstructionNode();
    equality_check->type = InstructionType::CJMP;
    equality_check->cjmp_inst.condition_op = ConditionalOperatorType::CONDITION_NOTEQUAL;
    equality_check->cjmp_inst.opernd1_index = var_location;
    expect(TokenType::CASE);

    // Get the location of the num
    int num_location = parse_constant(expect(TokenType::NUM));
    equality_check->cjmp_inst.opernd2_index = num_location;

    expect(TokenType::COLON);

    InstructionList body = parse_body();
    // When the equality check results to FALSE, it will jump to target. Thus, we need target to be the body
    equality_check->cjmp_inst.target = body.start;
    // Once we're done running the code in the body, we should go to the outside of the switch statement
    body.end->next = end_of_switch;

    // We're going to be linking the cases in parse_switch_stmt
    // So if the CJUMP evaluates to true (they aren't equal) we need to continue with the rest of the cases
    InstructionList result;
    result.start = equality_check;
    result.end = equality_check;
    return result;
}
// Parse and return default case
InstructionList Parser::parse_default_case(InstructionNode* end_of_switch) {
    expect(TokenType::DEFAULT);
    expect(TokenType::COLON);

    InstructionList body = parse_body();
    body.end->next = end_of_switch;
    return body;
}
// Parse inputs
void Parser::parse_inputs() {
    parse_num_list();
}
// Parse num list
void Parser::parse_num_list() {
    Token t = expect(TokenType::NUM);

    // Add constant to inputs list
    int val = std::stoi(t.lexeme);
    inputs.push_back(val);

    Token t1 = lexer.peek(1);
    if(t1.token_type != TokenType::END_OF_FILE) {
        parse_num_list();
    }
}