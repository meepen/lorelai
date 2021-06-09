#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include <iostream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;
using namespace lorelai::parser::expressions;

localassignmentstatement::localassignmentstatement(lexer &lex) {
	do {
		auto name = std::make_shared<nameexpression>(lex);
		left.push_back(name);
		children.push_back(name);
	}
	while (lex.read(","));

	if (!lex.read("=")) {
		return;
	}

	do {
		auto expr = expression::read(lex);
		if (!expr) {
			lex.wasexpected("<expression>", "assignment statement");
		}
		right.push_back(expr);
		children.push_back(expr);
	}
	while(lex.read(","));
}
