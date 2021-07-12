#include "statements.hpp"
#include "expressions.hpp"
#include "chunks.hpp"
#include "lexer.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;

whilestatement::whilestatement(lexer &lex) {
	lex.expect("while", "while .. do .. end");

	conditional = expression::read(lex);
	if (!conditional) {
		lex.wasexpected("<expression>", "while .. do .. end");
	}

	lex.expect("do", "while .. do .. end");

	block = new chunk(lex);

	lex.expect("end", "while .. do .. end");
}

string whilestatement::tostring() {
	std::stringstream stream;
	stream << "while " << conditional->tostring() << " do";

	// do we want to add the child nodes here?

	return stream.str();
}