#include "statements.hpp"
#include "expressions.hpp"
#include "chunks.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;

whilestatement::whilestatement(lexer &lex) {
	lex.expect("while", "while .. do .. end");

	conditional = expression::read(lex);
	children.push_back(conditional);

	lex.expect("do", "while .. do .. end");

	block = std::make_shared<chunk>(lex);
	children.push_back(block);

	lex.expect("end", "while .. do .. end");
}
