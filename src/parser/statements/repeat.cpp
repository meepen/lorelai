#include "statements.hpp"
#include "expressions.hpp"
#include "chunks.hpp"
#include "lexer.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;

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

string repeatstatement::tostring() {
	std::stringstream stream;
	stream << "repeat until " << conditional->tostring();

	// do we want to add the child nodes here?

	return stream.str();
}
