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

	while (auto stmt = statement::read(lex)) {
		statements.push_back(stmt);
		children.push_back(stmt);
	}

	word = lex.read();
	if (word != "end") {
		throw error::expected_for("end", "while .. do .. end", word);
	}
}
