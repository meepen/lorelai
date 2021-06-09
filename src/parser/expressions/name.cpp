#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;


nameexpression::nameexpression(lexer &lex) {
	name = lex.read();

	if (!lexer::isname(name)) {
		lex.wasexpected("<name>", "name");
	}
}