#include "expressions.hpp"
#include "lexer.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

enclosedexpression::enclosedexpression(lexer &lex) {
	lex.expect("(", "enclosed expression");
	enclosed = expression::read(lex);
	if (!enclosed) {
		lex.wasexpected("<expression>", "unclosed expression");
	}
	lex.expect(")", "enclosed expression");
}

string enclosedexpression::tostring() {
	std::stringstream stream;
	stream << "(" << enclosed->tostring() << ")";
	return stream.str();
}