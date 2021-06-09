#include "statements.hpp"
#include "expressions.hpp"
#include "chunks.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;

ifstatement::ifstatement(lexer &lex) {
	lex.expect("if", "if .. then .. end");

	conditional = expression::read(lex);
	if (!conditional) {
		lex.wasexpected("<expression>", "if .. then .. end");
	}
	children.push_back(conditional);

	lex.expect("then", "if .. then .. end");

	block = std::make_shared<chunk>(lex);
	children.push_back(block);

	lex.expect("end", "if .. then .. end");
}
