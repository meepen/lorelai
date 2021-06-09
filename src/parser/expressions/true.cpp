#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;


trueexpression::trueexpression(lexer &lex) {
	lex.expect("true", "true");
}