#include "expressions.hpp"
#include "lexer.hpp"
#include "args.hpp"
#include "visitor.hpp"
#include "funcbody.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

functionexpression::functionexpression(lexer &lex) {
	lex.expect("function", "function expression");
	body = new funcbody(lex);
}

LORELAI_ACCEPT_BRANCH(functionexpression)

string functionexpression::tostring() {
	return "function";
}