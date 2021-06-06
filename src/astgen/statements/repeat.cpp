#include "statements.hpp"
#include "expressions.hpp"
#include "errors.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;

repeatstatement::repeatstatement(lexer &lex) {
	// consume repeat
	lex.read();

	// try to read statement list
	while (auto stmt = statement::read(lex)) {
		statements.push_back(stmt);
		children.push_back(stmt);
	}

	auto ensure_until = lex.read();
	if (ensure_until != "until") {
		throw error::expected_for("end", "repeat .. until ..", lex.lookahead().value_or("no value"));
	}

	conditional = expression::read(lex);
	children.push_back(conditional);
}
