#include "args.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include "visitor.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

bool args::applicable(lexer &lex) {
	if (!lex.lookahead()) {
		return false;
	}

	if (stringexpression::applicable(lex)) {
		return true;
	}

	auto ahead = lex.lookahead().value();

	return ahead == "{" || ahead == "(";
}

args::args(lexer &lex) {
	if (!lex.lookahead()) {
		lex.wasexpected("<args>", "args parser");
	}
	auto ahead = lex.lookahead().value();

	// args ::=  `(´ [explist] `)´ | tableconstructor | String 

	if (ahead == "{") {
		auto arg = std::make_shared<tableexpression>(lex);
		children.push_back(arg);
	}
	else if (ahead == "(") {
		// arglist
		lex.expect("(", "args parser");
		while (true) {
			auto arg = expression::read(lex);
			if (!arg && children.size() == 0) {
				break;
			}

			children.push_back(arg);

			if (!lex.read(",")) {
				break;
			}
		}

		lex.expect(")", "args parser");
	}
	else {
		auto arg = std::make_shared<stringexpression>(lex);
		children.push_back(arg);
	}
}

LORELAI_VISIT_BRANCH_DEFINE(args)

string args::tostring() {
	std::stringstream stream;

	bool first = true;
	for (auto &arg : children) {
		if (!first) {
			stream << ", ";
		}
		first = false;
		stream << arg->tostring();
	}

	return stream.str();
}