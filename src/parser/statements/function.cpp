#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include "funcbody.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;
using namespace lorelai::parser::expressions;

functionstatement::functionstatement(lexer &lex) {
	lex.expect("function", "local function");

	name = std::make_shared<nameexpression>(lex);
	while (lex.read(".")) {
		std::shared_ptr<node> index = std::make_shared<nameexpression>(lex);
		name = std::make_shared<dotexpression>(name, index);
	}

	children.push_back(name);

	if (lex.read(":")) {
		method = std::make_shared<nameexpression>(lex);
		children.push_back(method);
	}

	body = std::make_shared<funcbody>(lex);
}

string functionstatement::tostring() {
	std::stringstream stream;
	stream << "function " << name->tostring();
	if (method) {
		stream << ":" << method->tostring();
	}

	return stream.str();
}