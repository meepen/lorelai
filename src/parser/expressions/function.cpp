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
	body = std::make_shared<funcbody>(lex);
}

LORELAI_VISIT_BRANCH_DEFINE(functionexpression)

string functionexpression::tostring() {
	return "function";
}