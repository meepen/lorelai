#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;


nilexpression::nilexpression(lexer &lex) {
	lex.expect("nil", "nil");
}