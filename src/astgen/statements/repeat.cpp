#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;

repeatstatement::repeatstatement(lexer &lex) {
	lex.expect("repeat", "repeat .. until ..");

	// try to read statement list
	while (auto stmt = statement::read(lex)) {
		statements.push_back(stmt);
		children.push_back(stmt);
	}

	lex.expect("until", "repeat .. until ..");

	conditional = expression::read(lex);
	children.push_back(conditional);
}
