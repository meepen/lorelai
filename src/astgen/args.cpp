#include "args.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include  "visitor.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;

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

bool args::accept(visitor &visit, std::shared_ptr<node> &container) {
	if (visit.visit(*this, container)) {
		return true;
	}

	visitchildren(visit);
	return false;
}