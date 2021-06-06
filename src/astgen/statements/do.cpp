#include "statements.hpp"
#include "expressions.hpp"
#include "errors.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;

dostatement::dostatement(lexer &lex) {
	// consume do
	lex.read();

	// try to read statement list
	while (lex.lookahead() && lex.lookahead().value() != "end") {
		auto stmt = statement::read(lex);
		if (!stmt) {
			throw error::expected_for("statement", "do .. end", lex.lookahead().value_or("no value"));
		}

		statements.push_back(stmt);
		children.push_back(stmt);
	}

	auto ensure_end = lex.read();

	if (ensure_end != "end") {
		throw error::expected_for("end", "do .. end", lex.lookahead().value_or("no value"));
	}
}
