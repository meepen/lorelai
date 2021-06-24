#include "statements.hpp"
#include "lexer.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;

breakstatement::breakstatement(lexer &lex) {
	lex.expect("break", "break");
}

LORELAI_VISIT_NODE_DEFINE(breakstatement)