#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include "funcbody.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;
using namespace lorelai::parser::expressions;

functionstatement::functionstatement(lexer &lex) {
	lex.expect("function", "local function");

	auto name = std::make_shared<nameexpression>(lex);
	names.push_back(name);
	children.push_back(name);
	while (lex.read(".")) {
		name = std::make_shared<nameexpression>(lex);
		names.push_back(name);
		children.push_back(name);
	}

	if (lex.read(":")) {
		method = std::make_shared<nameexpression>(lex);
		children.push_back(name);
	}

	body = std::make_shared<funcbody>(lex);
}