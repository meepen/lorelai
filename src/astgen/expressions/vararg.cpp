#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;


varargexpression::varargexpression(lexer &lex) {
	lex.expect("...", "...");
}