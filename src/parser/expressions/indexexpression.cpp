#include "expressions.hpp"
#include "lexer.hpp"
#include "visitor.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;


indexexpression::indexexpression(node *_prefix, lexer &lex) : prefix(_prefix) {
	lex.expect("[", "index expression");

	index = expression::read(lex);
	if (!index) {
		lex.wasexpected("<expression>", "index expression");
	}

	lex.expect("]", "index expression");
}

dotexpression::dotexpression(node *_prefix, lexer &lex) : prefix(_prefix) {
	lex.expect(".", "dot expression");

	index = nameexpression(lex).name;
}

LORELAI_ACCEPT_BRANCH(indexexpression)
LORELAI_ACCEPT_BRANCH(dotexpression)

string dotexpression::tostring() {
	std::stringstream stream;
	stream << prefix->tostring() << "." << index;

	return stream.str();
}

string indexexpression::tostring() {
	std::stringstream stream;
	stream << prefix->tostring() << "[" << index->tostring() << "]";

	return stream.str();
}