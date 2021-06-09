#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;


nilexpression::nilexpression(lexer &lex) {
	lex.expect("nil", "nil");
}