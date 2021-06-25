#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;


varargexpression::varargexpression(lexer &lex) {
	lex.expect("...", "...");
}

string varargexpression::tostring() {
	return "...";
}