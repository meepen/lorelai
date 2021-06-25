#include "expressions.hpp"
#include "lexer.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

enclosedexpression::enclosedexpression(lexer &lex) {
	lex.expect("(", "enclosed expression");
	children.push_back(expression::read(lex));
	lex.expect(")", "enclosed expression");
}

string enclosedexpression::tostring() {
	std::stringstream stream;
	stream << "(" << children[0]->tostring() << ")";
	return stream.str();
}