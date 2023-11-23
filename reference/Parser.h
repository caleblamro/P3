
/*
Abel Haddis and I worked on this together before the solo project rule, Got permission to submit together
We worked on the code together
 */

#ifndef __PARSER_H__
#define __PARSER_H__
#include <map>
#include <list>
#include <string>
#include <set>
#include <vector>
#include "lexer.h"
#include "compiler.h"
using namespace std;

struct parseExprResult {
	int LeftMem;
	ArithmeticOperatorType op;
	int RightMem;
};
struct codeBlock {
	InstructionNode* start;
	InstructionNode* end;
};
struct CaseBlock {
	codeBlock body;
	int memNum;
};
struct ParseResult {
	InstructionNode* program;
	std::map<int, int> StartingMemory;
	std::vector<int> inputs;

};
#pragma once
class Parser {
public:
	Parser();
	ParseResult parse_program();


private:


	std::vector<int> inputs;
	std::map<std::string, int> VariableLocations;
	std::map<int, int> StartingMemory;
	//vector <variables*> variables;

	//Map Stuff
	void addVarToMap(std::string);
	int getVarLocation(std::string);
	int addConstantToMap(int);
	int NextMemLoction;


	int stringToInt(std::string);

	void parse_var_section();
	void parse_id_list();
	codeBlock parse_body();
	codeBlock parse_stmt_list();
	codeBlock parse_stmt();
	InstructionNode* parse_assign_stmt();
	parseExprResult parse_expr();
	int parse_primary();
	ArithmeticOperatorType parse_op();
	InstructionNode* parse_output_stmt();
	InstructionNode* parse_input_stmt();
	codeBlock parse_while_stmt();
	codeBlock parse_if_stmt();
	InstructionNode* createNOOP();
	InstructionNode* parse_condition();
	ConditionalOperatorType parse_relop();
	codeBlock parse_switch_stmt();
	codeBlock parse_for_stmt();
	std::list <CaseBlock> parse_case_list();
	CaseBlock parse_case();
	codeBlock parse_default_case();
	void parse_inputs();
	void parse_num_list();
	Token expect(TokenType);
	//Token expectOutputId(TokenType);
	void syntax_error();
	LexicalAnalyzer lexer;
	codeBlock caseToCode(CaseBlock, int, InstructionNode*);
};

#endif
