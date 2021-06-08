#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;
using namespace lorelai::astgen::expressions;

localsstatement::localsstatement(lexer &lex) {
	// `local` was already consumed by the local deducer

	while (true) {
		auto name = std::make_shared<nameexpression>(lex);

		names.push_back(name);
		children.push_back(name);

		if (!lex.lookahead() || lex.lookahead().value() != ",") {
			break;
		}

		lex.expect(",", "locals statement");
	}

	if (lex.lookahead() && lex.lookahead().value() == "=") {
		lex.expect("=", "locals statement");
		// initializer

		while (true) {
			auto expr = expression::read(lex);
			if (!expr) {
				lex.wasexpected("<expression>", "locals statement");
			}

			initializers.push_back(expr);
			children.push_back(expr);

			if (!lex.lookahead() || lex.lookahead().value() != ",") {
				break;
			}

			lex.expect(",", "locals statement");
		}
	}
}