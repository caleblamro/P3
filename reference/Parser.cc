/*
Abel Haddis and I worked on this together before the solo project rule, Got permission to submit together
We worked on the code together
 */
#include "Parser.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <sstream>
#include <string>
#include <iostream>
#include "compiler.h"

using namespace std;
Parser::Parser() {
    NextMemLoction = 0;
}

Token Parser::expect(TokenType expectedTokenType) {
    Token t = lexer.GetToken();
    if (expectedTokenType == t.token_type) {
        return t;
    }
    syntax_error();
    return t;
}
void Parser::syntax_error() {
    cout << "SYNTAX ERROR";
    exit(1);
}

InstructionNode* Parser::parse_assign_stmt() {
    Token Vartoken = expect(ID);
    InstructionNode* assignstmt = new InstructionNode;
    int IDmemADR = getVarLocation(Vartoken.lexeme);
    expect(EQUAL);
    Token t = lexer.peek(1);
    Token secondPeek = lexer.peek(2);
    if (t.token_type == ID || t.token_type == NUM) {
        if (secondPeek.token_type == PLUS || secondPeek.token_type == MINUS || secondPeek.token_type == MULT || secondPeek.token_type == DIV) {
            parseExprResult MATH = parse_expr();

            assignstmt->type = InstructionType::ASSIGN;
            assignstmt->assign_inst.left_hand_side_index = IDmemADR;
            assignstmt->assign_inst.operand1_index = MATH.LeftMem;
            assignstmt->assign_inst.operand2_index = MATH.RightMem;
            assignstmt->assign_inst.op = MATH.op;
            assignstmt->next = NULL;
            expect(SEMICOLON);
            return assignstmt;

        }
        else if (secondPeek.token_type == SEMICOLON) {
            int memADR = parse_primary();
            assignstmt->type = InstructionType::ASSIGN;
            assignstmt->assign_inst.left_hand_side_index = IDmemADR;
            assignstmt->assign_inst.operand1_index = memADR;
            assignstmt->assign_inst.op = OPERATOR_NONE;
            assignstmt->next = NULL;
            expect(SEMICOLON);
            return assignstmt;
        }
    }
    return NULL;
}

codeBlock Parser::parse_body() {
    expect(LBRACE);
    codeBlock liststatement = parse_stmt_list();
    expect(RBRACE);
    return liststatement;
}


CaseBlock Parser::parse_case() {
    CaseBlock result;
    Token Numstuff;
    expect(CASE);
    Numstuff = expect(NUM);
    result.memNum = addConstantToMap(stringToInt(Numstuff.lexeme));
    expect(COLON);
    result.body = parse_body();
    return result;
}


list<CaseBlock> Parser::parse_case_list() {
    list <CaseBlock> result;
    CaseBlock top;
    top = parse_case();
    Token t = lexer.peek(1);
    if (t.token_type == CASE) {
        list<CaseBlock> newResult;
        newResult = parse_case_list();
        newResult.push_front(top);
        return newResult;
    }
    result.push_front(top);
    return result;
}

InstructionNode* Parser::parse_condition() {
    // ID/NUM >< ID/NUM
    InstructionNode* conditionNode = new InstructionNode;
    conditionNode->cjmp_inst.operand1_index = parse_primary();
    conditionNode->cjmp_inst.condition_op = parse_relop();
    conditionNode->cjmp_inst.operand2_index = parse_primary();
    conditionNode->type = InstructionType::CJMP;
    conditionNode->next = NULL;
    return conditionNode;
}

codeBlock Parser::parse_default_case() {
    //codeBlock result;
    expect(DEFAULT);
    expect(COLON);
    return parse_body();
}

parseExprResult Parser::parse_expr() {
    parseExprResult math;
    math.LeftMem = parse_primary();
    math.op = parse_op();
    math.RightMem = parse_primary();
    return math;
}


codeBlock Parser::parse_for_stmt() {
    codeBlock result;
    InstructionNode* Assign1Node;
    InstructionNode* Assign2Node;
    InstructionNode* conditionNode;
    codeBlock bodyNodes;
    InstructionNode* Noooop;
    // InstructionNode* stuff;
    Noooop = createNOOP();
    expect(FOR);
    expect(LPAREN);
    Assign1Node = parse_assign_stmt();
    conditionNode = parse_condition();
    expect(SEMICOLON);
    Assign2Node = parse_assign_stmt();
    expect(RPAREN);
    bodyNodes = parse_body();
    result.start = Assign1Node;
    Assign1Node->next = conditionNode;
    conditionNode->cjmp_inst.target = Noooop;
    conditionNode->next = bodyNodes.start;
    bodyNodes.end->next = Assign2Node;
    InstructionNode* jumper = new InstructionNode;
    jumper->jmp_inst.target = conditionNode;
    jumper->type = InstructionType::JMP;
    jumper->next = NULL;
    Assign2Node->next = jumper;
    jumper->next = Noooop;

    result.end = Noooop;

    return result;
}

void Parser::parse_id_list() {
    Token GottedToken = expect(ID);
    addVarToMap(GottedToken.lexeme);

    Token t = lexer.peek(1);
    if (t.token_type == COMMA) {
        expect(COMMA);
        parse_id_list();
    }
}

codeBlock Parser::parse_if_stmt() {
    //
    expect(IF);
    InstructionNode* ifNode;
    codeBlock BodyNode;
    codeBlock result;
    InstructionNode* Noop = createNOOP();

    ifNode = parse_condition();
    ifNode->cjmp_inst.target = Noop;

    BodyNode = parse_body();
    ifNode->next = BodyNode.start;
    // BodyNode.start->next = BodyNode.end;

    BodyNode.end->next = Noop;
    result.start = ifNode;
    result.end = Noop;
    return result;

}

void Parser::parse_inputs() {
    parse_num_list();
}

InstructionNode* Parser::parse_input_stmt() {
    expect(INPUT);
    Token VariableToken = expect(ID);
    expect(SEMICOLON);
    InstructionNode* inputstatement = new InstructionNode;
    inputstatement->type = InstructionType::IN;
    inputstatement->input_inst.var_index = getVarLocation(VariableToken.lexeme);
    inputstatement->next = NULL;
    return inputstatement;
}

void Parser::parse_num_list() {
    Token gotted = expect(NUM);
    int NumVal = stringToInt(gotted.lexeme);
    inputs.push_back(NumVal);
    Token t = lexer.peek(1);
    if (t.token_type == NUM) {
        parse_num_list();
    }

}

ArithmeticOperatorType Parser::parse_op() {
    Token t = lexer.peek(1);


    if (t.token_type == PLUS) {
        expect(PLUS);
        return OPERATOR_PLUS;
    }
    if (t.token_type == MINUS) {
        expect(MINUS);
        return OPERATOR_MINUS;
    }
    if (t.token_type == MULT) {
        expect(MULT);
        return OPERATOR_MULT;
    }
    if (t.token_type == DIV) {
        expect(DIV);
        return OPERATOR_DIV;
    }
    syntax_error();
    exit(1);
}

InstructionNode* Parser::parse_output_stmt() {

    expect(OUTPUT);
    Token VariableToken = expect(ID);
    expect(SEMICOLON);

    InstructionNode* outputstatement = new InstructionNode;
    outputstatement->type = InstructionType::OUT;
    outputstatement->output_inst.var_index = getVarLocation(VariableToken.lexeme);
    outputstatement->next = NULL;
    return outputstatement;
}

int Parser::parse_primary() {
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        Token IDToken = expect(ID);
        return getVarLocation(IDToken.lexeme);
    }
    Token NumToken = expect(NUM);
    int Value = stringToInt(NumToken.lexeme);
    return addConstantToMap(Value);
}

ParseResult Parser::parse_program() {
    ParseResult Result;
    Result.program = NULL;

    parse_var_section();
    Result.program = parse_body().start;
    parse_inputs();
    Result.StartingMemory = this->StartingMemory;
    Result.inputs = this->inputs;
    return Result;
}

codeBlock Parser::parse_stmt() {
    codeBlock result;
    result.start = NULL;
    result.end = NULL;
    InstructionNode* GrabbedInstruction;
    //codeBlock grabbedBlock;
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        GrabbedInstruction = parse_assign_stmt();
        result.start = GrabbedInstruction;
        result.end = GrabbedInstruction;
        return result;
    }
    if (t.token_type == WHILE) {
        result = parse_while_stmt();
        return result;
    }
    if (t.token_type == IF) {

        result = parse_if_stmt();

        return result;
    }

    if (t.token_type == SWITCH) {
        result = parse_switch_stmt();
        return result;
    }
    if (t.token_type == FOR) {
        result = parse_for_stmt();
        return result;
    }
    if (t.token_type == OUTPUT) {
        GrabbedInstruction = parse_output_stmt();
        result.start = GrabbedInstruction;
        result.end = GrabbedInstruction;
        return result;
    }
    if (t.token_type == INPUT) {
        GrabbedInstruction = parse_input_stmt();
        result.start = GrabbedInstruction;
        result.end = GrabbedInstruction;
        return result;
    }
    return result;
}
ConditionalOperatorType Parser::parse_relop() {
    Token t = lexer.peek(1);
    if (t.token_type == GREATER) {
        expect(GREATER);
        return CONDITION_GREATER;
    }
    if (t.token_type == LESS) {
        expect(LESS);
        return CONDITION_LESS;

    }
    if (t.token_type == NOTEQUAL) {
        expect(NOTEQUAL);
        return CONDITION_NOTEQUAL;
    }
    return  CONDITION_NOTEQUAL;
}

codeBlock Parser::parse_stmt_list() {

    codeBlock statment = parse_stmt();
    Token t = lexer.peek(1);
    if (t.token_type == ID || t.token_type == WHILE || t.token_type == IF || t.token_type == SWITCH || t.token_type == FOR || t.token_type == INPUT || t.token_type == OUTPUT) {
        codeBlock list = parse_stmt_list();
        statment.end->next = list.start;
        codeBlock result;
        result.start = statment.start;
        result.end = list.end;
        return result;

    }

    return statment;
}
codeBlock Parser::parse_switch_stmt() {
    codeBlock result;
    InstructionNode* noopNode;
    noopNode = createNOOP();
    list<CaseBlock> cases;
    codeBlock defaultcase;
    list<codeBlock> part2cases;
    Token idStuff;
    int idmem;
    expect(SWITCH);
    idStuff = expect(ID);
    idmem = getVarLocation(idStuff.lexeme);
    expect(LBRACE);
    cases = parse_case_list();
    Token t = lexer.peek(1);
    for (list<CaseBlock>::iterator itr = cases.begin(); itr != cases.end(); ++itr) {
        part2cases.push_back(caseToCode(*itr, idmem, noopNode));
    }
    list<codeBlock>::iterator itr1 = part2cases.begin();
    list<codeBlock>::iterator itr2 = part2cases.begin();
    ++itr2;
    while (itr2 != part2cases.end()) {
        itr1->end->next = itr2->start;
        itr1++;
        itr2++;
    }
    result.start = part2cases.front().start;
    result.end = part2cases.back().end;


    if (t.token_type == DEFAULT) {
        defaultcase = parse_default_case();
        result.end->next = defaultcase.start;
        result.end = defaultcase.end;

    }
    result.end->next = noopNode;
    result.end = noopNode;
    expect(RBRACE);


    return result;
}
codeBlock Parser::caseToCode(CaseBlock trial, int varLocation, InstructionNode* NOOP) {
    codeBlock result;
    InstructionNode* assignstmt = new InstructionNode;
    InstructionNode* condionNode = new InstructionNode;
    InstructionNode* nooppart2 = createNOOP();
    InstructionNode* jumper = new InstructionNode;
    jumper->jmp_inst.target = NOOP;
    jumper->type = InstructionType::JMP;
    jumper->next = nooppart2;

    assignstmt->type = InstructionType::ASSIGN;
    assignstmt->assign_inst.left_hand_side_index = addConstantToMap(0);
    assignstmt->assign_inst.operand1_index = varLocation;
    assignstmt->assign_inst.operand2_index = trial.memNum;
    assignstmt->assign_inst.op = OPERATOR_MINUS;
    assignstmt->next = condionNode;
    condionNode->type = InstructionType::CJMP;
    condionNode->cjmp_inst.target = trial.body.start;
    condionNode->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    condionNode->cjmp_inst.operand1_index = addConstantToMap(0);
    condionNode->cjmp_inst.operand2_index = assignstmt->assign_inst.left_hand_side_index;
    condionNode->next = nooppart2;
    trial.body.end->next = jumper;

    result.start = assignstmt;
    result.end = nooppart2;
    return result;

}

void Parser::parse_var_section() {
    parse_id_list();
    expect(SEMICOLON);
}
codeBlock Parser::parse_while_stmt() {
    InstructionNode* whileNode;
    codeBlock BodyNode;
    codeBlock result;
    InstructionNode* Noop;
    Noop = createNOOP();
    expect(WHILE);
    whileNode = parse_condition();
    whileNode->cjmp_inst.target = Noop;


    InstructionNode* jumper = new InstructionNode;
    jumper->jmp_inst.target = whileNode;
    jumper->type = InstructionType::JMP;
    jumper->next = NULL;
    //   BodyNode = 
    BodyNode = parse_body();
    whileNode->next = BodyNode.start;
    BodyNode.end->next = jumper;
    result.start = whileNode;
    result.end = Noop;
    return result;
}


int Parser::stringToInt(string s) {
    stringstream ss;
    ss << s;
    int result;
    ss >> result;
    return result;
}
InstructionNode* Parser::createNOOP() {
    InstructionNode* NOOP = new InstructionNode;
    NOOP->type = InstructionType::NOOP;
    NOOP->next = NULL;
    return NOOP;
}

//map<std::string, int> VariableLocations;
// NextMemLoction
void Parser::addVarToMap(string VarName) {
    VariableLocations[VarName] = NextMemLoction;
    StartingMemory[NextMemLoction] = 0;
    NextMemLoction = NextMemLoction + 1;
    return;
}
int Parser::getVarLocation(string VarName) {
    return VariableLocations.at(VarName);
}

int Parser::addConstantToMap(int value) {
    int memofConstant;
    StartingMemory[NextMemLoction] = value;
    memofConstant = NextMemLoction;
    NextMemLoction++;
    return memofConstant;
}
