#include "statements.hpp"
#include "expressions.hpp"
#include "errors.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;

whilestatement::whilestatement(lexer &lex) {
	// consume while
	lex.read();
	conditional = expression::read(lex);
	children.push_back(conditional);

	auto word = lex.read();
	if (word != "do") {
		throw error::expected_for("do", "while .. do .. end", word);
	}

	// try to read statement list
	while (lex.lookahead() && lex.lookahead().value() != "end") {
		auto stmt = statement::read(lex);
		if (!stmt) {
			throw error::expected_for("statement", "while .. do .. end", lex.lookahead().value_or("no value"));
		}

		statements.push_back(stmt);
		children.push_back(stmt);
	}

	word = lex.read();
	if (word != "end") {
		throw error::expected_for("end", "while .. do .. end", word);
	}
}
