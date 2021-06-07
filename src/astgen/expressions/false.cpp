#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;


falseexpression::falseexpression(lexer &lex) {
	lex.expect("false", "false");
}