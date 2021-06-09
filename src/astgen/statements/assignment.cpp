#include "statements.hpp"
#include "visitor.hpp"
#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;
using namespace lorelai::astgen::expressions;

assignmentstatement::assignmentstatement(std::shared_ptr<node> prefix, lexer &lex) {
	left.push_back(prefix);
	children.push_back(prefix);

	while (lex.read(",")) {
		auto lvalue = expression::read(lex);
		if (!lvalue || !dynamic_cast<varexpression *>(lvalue.get())) {
			lex.wasexpected("<lvalue>", "assignment statement");
		}
		left.push_back(lvalue);
		children.push_back(lvalue);
	}

	if (!lex.read("=")) {
		lex.wasexpected("=", "assignment statement");
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
