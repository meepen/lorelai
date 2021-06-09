#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;


falseexpression::falseexpression(lexer &lex) {
	lex.expect("false", "false");
}