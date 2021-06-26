#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

numberexpression::numberexpression(lexer &lex) {
	data = tonumber(lex.read());
}

string numberexpression::tostring() {
	return std::to_string(data);
}