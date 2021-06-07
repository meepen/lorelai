#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;


trueexpression::trueexpression(lexer &lex) {
	lex.expect("true", "true");
}