#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include "funcbody.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;
using namespace lorelai::astgen::expressions;

localfunctionstatement::localfunctionstatement(lexer &lex) {
	// `local` is already consumed by the local deducer

	lex.expect("function", "local function");

	name = std::make_shared<nameexpression>(lex);
	children.push_back(name);
	body = std::make_shared<funcbody>(lex);
	children.push_back(body);
}