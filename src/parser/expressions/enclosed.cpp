#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

enclosedexpression::enclosedexpression(lexer &lex) {
	lex.expect("(", "enclosed expression");
	children.push_back(expression::read(lex));
	lex.expect(")", "enclosed expression");
}