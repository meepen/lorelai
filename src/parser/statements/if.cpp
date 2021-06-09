#include "statements.hpp"
#include "expressions.hpp"
#include "chunks.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;

ifstatement::ifstatement(lexer &lex) {
	lex.expect("if", "if .. then .. end");

	auto conditional = expression::read(lex);
	if (!conditional) {
		lex.wasexpected("<expression>", "if .. then .. end");
	}
	children.push_back(conditional);
	conditionals.push_back(conditional);

	lex.expect("then", "if .. then .. end");

	auto block = std::make_shared<chunk>(lex);
	children.push_back(block);
	blocks.push_back(block);

	while (lex.read("elseif")) {
		conditional = expression::read(lex);
		if (!conditional) {
			lex.wasexpected("<expression>", "if .. then .. elseif .. end");
		}
		conditionals.push_back(conditional);
		children.push_back(conditional);

		lex.expect("then", "if .. then .. elseif .. end");
		
		block = std::make_shared<chunk>(lex);
		children.push_back(block);
		blocks.push_back(block);

	}

	if (lex.read("else")) {
		block = std::make_shared<chunk>(lex);
		children.push_back(block);
		blocks.push_back(block);
	}

	lex.expect("end", "if .. then .. end");
}
