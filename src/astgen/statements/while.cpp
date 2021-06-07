#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;

whilestatement::whilestatement(lexer &lex) {
	lex.expect("while", "while .. do .. end");

	conditional = expression::read(lex);
	children.push_back(conditional);

	lex.expect("do", "while .. do .. end");

	while (auto stmt = statement::read(lex)) {
		statements.push_back(stmt);
		children.push_back(stmt);
	}

	lex.expect("end", "while .. do .. end");
}
