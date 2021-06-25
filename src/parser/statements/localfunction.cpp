#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include "funcbody.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;
using namespace lorelai::parser::expressions;

localfunctionstatement::localfunctionstatement(lexer &lex) {
	// `local` is already consumed by the local deducer

	lex.expect("function", "local function");

	name = std::make_shared<nameexpression>(lex);
	children.push_back(name);

	body = std::make_shared<funcbody>(lex);
	children.push_back(body);
}

string localfunctionstatement::tostring() {
	std::stringstream stream;
	stream << "local function " << name->tostring();

	// do we want to add the child nodes here?

	return stream.str();
}