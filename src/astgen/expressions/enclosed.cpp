#include "expressions.hpp"
#include "lexer.hpp"
#include "errors.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;


enclosedexpression::enclosedexpression(lexer &lex) {
	lex.expect("(", "enclosed expression");
	children.push_back(expression::read(lex));
	lex.expect(")", "enclosed expression");
}