#include "parser.h"
#include <iostream>

// Although it will not be needed, this will be used in spots that would cause an error, since there are no errors, it will never be called
void syntax_error() {
    std::cout << "SYNTAX ERROR!!!" << std::endl;
    exit(0);
}

int Parser::location(std::string id) {
    auto it = program.mem_mappings.find(id);
    if(it == program.mem_mappings.end()) {
        return -1;
    }else{
        return program.mem_mappings[id];
    }
}

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
        std::cout << "Error: expected: " << t << " but found: " << tkn.lexeme << " type: " << tkn.token_type << std::endl;
        syntax_error();
    }
    return tkn;
}

Parser::Parser() {
    variable_counter = 0;
}

int Parser::parse_constant(Token t) {
    int result = location(t.lexeme);

    if(result == -1) {
        program.mem_mappings[t.lexeme] = variable_counter;
        // Assign the constant value to the memory location
        program.mem[variable_counter++] = std::stoi(t.lexeme);
    }
    return location(t.lexeme);
}

Program Parser::parse_program() {
    // std::cout << "program" << std::endl;
    parse_var_section();
    // At this point, we've populated program.mem_mappings with all the variable names declared in var_section
    InstructionList body = parse_body();
    program.start = body.start;

    parse_inputs();

    return program;
}

void Parser::parse_var_section() {
    // std::cout << "var_section" << std::endl;
    parse_id_list();
    expect(TokenType::SEMICOLON);
}

void Parser::parse_id_list() {
    // std::cout << "id_list" << std::endl;
    Token t = expect(TokenType::ID);
    // No need to check if we've inserted a name already, instructions say to assume all names are unique
    program.mem_mappings[t.lexeme] = variable_counter;
    //Initialize this variable to zero
    program.mem[variable_counter++] = 0;
    Token maybe_comma = lexer.peek(1);
    if(maybe_comma.token_type == TokenType::COMMA) {
        lexer.GetToken();
        parse_id_list();
    }
}

InstructionList Parser::parse_body() {
    // std::cout << "body" << std::endl;
    expect(TokenType::LBRACE);

    InstructionList list = parse_stmt_list();

    expect(TokenType::RBRACE);
    return list;
}

InstructionList Parser::parse_stmt_list() {
    // std::cout << "stmt_list" << std::endl;
    InstructionList parsed_stmt = parse_stmt();
    // we have our statement, but we need to ensure that we know where the beginning and end of the stmt is
    Token t = lexer.peek(1);
    // We know this is the last stmt if after the stmt we see an RBRACE, which closes out the stmt_list
    if(t.token_type == TokenType::RBRACE) {
        return parsed_stmt;
    }
    // This is quite verbose, but it simply checks if t.token_type is in FIRST(stmt)
    if(t.token_type == TokenType::ID || t.token_type == TokenType::WHILE || t.token_type == TokenType::IF|| t.token_type == TokenType::SWITCH 
    || t.token_type == TokenType::FOR || t.token_type == TokenType::OUTPUT || t.token_type == TokenType::INPUT) {
        // At this point, we know we have another statement to parse, so we can call parse_stmt_list recursively
        InstructionList parsed_list = parse_stmt_list();
        parsed_stmt.end->next = parsed_list.start;
        InstructionList result;
        result.start = parsed_stmt.start;
        result.end = parsed_list.end;
        return result;
    }
    syntax_error();
}

InstructionList Parser::parse_stmt() {
    // std::cout << "stmt" << std::endl;
    Token t = lexer.peek(1);
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

InstructionList Parser::parse_assign_stmt() {
    // std::cout << "assign_stmt" << std::endl;
    // Create empty assign stmt
    InstructionNode* stmt = new InstructionNode();
    stmt->type = InstructionType::ASSIGN;
    stmt->next = NULL;
    // Extract LHS variable info
    Token var_id = expect(TokenType::ID);
    int var_location = location(var_id.lexeme);

    stmt->assign_inst.left_hand_side_index = var_location;

    expect(TokenType::EQUAL);

    // Peek the next token, to check which non-terminal (primary or expr) should be parsed - don't consume since parse_primary and parse_expr do this
    Token op = lexer.peek(2);
    // primary has only two possibilities: ID or NUM, so we know that if we see and ID or NUM primary must be the next non-terminal
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

Expression Parser::parse_expr() {
    // std::cout << "expr" << std::endl;
    int op1_location = parse_primary();
    ArithmeticOperatorType op_type = parse_op();
    int op2_location = parse_primary();
    Expression e;
    e.op1_location = op1_location;
    e.op_type = op_type;
    e.op2_location = op2_location;
    return e;
}

int Parser::parse_primary() {
    // std::cout << "primary" << std::endl;
    Token t = lexer.GetToken();
    if(t.token_type == TokenType::ID) {
        return location(t.lexeme);
    }else if(t.token_type == TokenType::NUM) {
        return parse_constant(t);
    }
}

ArithmeticOperatorType Parser::parse_op() {
    // std::cout << "op" << std::endl;
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

InstructionList Parser::parse_output_stmt() {
    // std::cout << "output_stmt" << std::endl;
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

InstructionList Parser::parse_input_stmt() {
    // std::cout << "input_stmt" << std::endl;
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

InstructionList Parser::parse_while_stmt() {
    // std::cout << "while_stmt" << std::endl;
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
    stmt->cjmp_inst.target = no_operation;


    // Create the jump node which jumps back to the conditional jump
    InstructionNode* jump = new InstructionNode();
    jump->type = InstructionType::JMP;
    jump->jmp_inst.target = stmt;


    InstructionList body = parse_body();
    stmt->next = body.start;
    body.end->next = jump;


    InstructionList result;
    result.start = stmt;
    result.end = no_operation;
    return result;
}

InstructionList Parser::parse_if_stmt() {
    // std::cout << "if_stmt" << std::endl;
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
    // std::cout << "condition" << std::endl;
    Condition c;
    
    int op1_location = parse_primary();
    ConditionalOperatorType op_type = parse_relop();
    int op2_location = parse_primary();

    c.op1_location = op1_location;
    c.type = op_type;
    c.op2_location = op2_location;

    return c;
}

ConditionalOperatorType Parser::parse_relop() {
    // std::cout << "relop" << std::endl;
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

InstructionList Parser::parse_switch_stmt() {
    expect(TokenType::SWITCH);
    
    int var_location = location(expect(TokenType::ID).lexeme);

    expect(TokenType::LBRACE);
    
    // Set up a location for the 
    InstructionNode* end_of_switch = get_no_operation();
    
    std::vector<InstructionList> cases;
    parse_case_list(var_location, end_of_switch, cases);

    Token t = lexer.peek(1);
    if(t.token_type == TokenType::DEFAULT) {
        InstructionList default_case = parse_default_case(end_of_switch);
        for(size_t i = 0; i < cases.size() - 1; i++) {
            cases[i].start->next = cases[i+1].start;
        }
        cases[cases.size() - 1].start->next = default_case.start;
    }else{
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

InstructionList Parser::parse_for_stmt() {
    // std::cout << "for_stmt" << std::endl;
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
    InstructionNode* assign_stmt_repeat = parse_assign_stmt().start;
    expect(TokenType::RPAREN);

    InstructionList body = parse_body();

    InstructionNode* jump = new InstructionNode();
    jump->type = InstructionType::JMP;
    jump->jmp_inst.target = condition_stmt;

    assign_stmt->next = condition_stmt;
    condition_stmt->next = body.start;
    body.end->next = assign_stmt_repeat;
    assign_stmt_repeat->next = jump;

    InstructionList result;
    result.start = assign_stmt;
    result.end = end_of_for;
    return result;
}

void Parser::parse_case_list(int var_location, InstructionNode* end_of_switch, std::vector<InstructionList>& list) {
    // std::cout << "case_list" << std::endl;
    InstructionList parsed_case = parse_case(var_location, end_of_switch);
    list.push_back(parsed_case);
    Token t = lexer.peek(1);
    if(t.token_type == TokenType::CASE) {
        parse_case_list(var_location, end_of_switch, list);
    }
}

InstructionList Parser::parse_case(int var_location, InstructionNode* end_of_switch) {
    // std::cout << "case" << std::endl;
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

InstructionList Parser::parse_default_case(InstructionNode* end_of_switch) {
    // std::cout << "default_case" << std::endl;
    expect(TokenType::DEFAULT);
    expect(TokenType::COLON);

    InstructionList body = parse_body();
    body.end->next = end_of_switch;
    return body;
}

void Parser::parse_inputs() {
    // std::cout << "inputs" << std::endl;
    parse_num_list();
}

void Parser::parse_num_list() {
    // std::cout << "num_list" << std::endl;
    Token t = expect(TokenType::NUM);

    int val = std::stoi(t.lexeme);
    inputs.push_back(val);

    Token t1 = lexer.peek(1);
    if(t1.token_type != TokenType::END_OF_FILE) {
        parse_num_list();
    }
}