#include "statements.hpp"
#include "expressions.hpp"
#include "chunks.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;

repeatstatement::repeatstatement(lexer &lex) {
	lex.expect("repeat", "repeat .. until ..");

	block = std::make_shared<chunk>(lex);
	children.push_back(block);

	lex.expect("until", "repeat .. until ..");

	conditional = expression::read(lex);
	if (!conditional) {
		lex.wasexpected("<expression>", "repeat .. until ..");
	}
	children.push_back(conditional);
}
