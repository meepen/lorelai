#include "statements.hpp"
#include "visitor.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;
using namespace lorelai::parser::expressions;

assignmentstatement::assignmentstatement(node *prefix, lexer &lex) {
	left.push_back(prefix);

	while (lex.read(",")) {
		auto lvalue = expression::read(lex);
		if (!lvalue || !dynamic_cast<varexpression *>(lvalue)) {
			lex.wasexpected("<lvalue>", "assignment statement");
		}
		left.push_back(lvalue);
	}

	lex.expect("=", "assignment statement");

	do {
		auto expr = expression::read(lex);
		if (!expr) {
			lex.wasexpected("<expression>", "assignment statement");
		}
		right.push_back(expr);
	}
	while(lex.read(","));
}

string assignmentstatement::tostring() {
	std::stringstream stream;
	bool first = true;
	for (auto &lhs : left) {
		if (!first) {
			stream << ", ";
		}
		first = false;
		stream << lhs->tostring();
	}

	if (right.size() > 0) {
		stream << " = ";

		first = true;
		for (auto &rhs : right) {
			if (!first) {
				stream << ", ";
			}
			first = false;
			stream << rhs->tostring();
		}
	}

	return stream.str();
}