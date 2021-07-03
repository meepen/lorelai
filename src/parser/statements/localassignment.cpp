#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include <iostream>
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;
using namespace lorelai::parser::expressions;

localassignmentstatement::localassignmentstatement(lexer &lex) {
	do {
		left.push_back(nameexpression(lex).name);
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

string localassignmentstatement::tostring() {
	std::stringstream stream;
	stream << "local ";

	bool first = true;

	for (auto &child : left) {
		if (!first) {
			stream << ", ";
		}
		first = false;
		stream << child;
	}
	
	if (right.size() > 0) {
		first = true;
		stream << " = ";

		for (auto &child : right) {
			if (!first) {
				stream << ", ";
			}
			first = false;
			stream << child->tostring();
		}
	}

	return stream.str();
}