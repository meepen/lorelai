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
	lex.expect("function", "function");

	name = std::make_shared<nameexpression>(lex);
	while (lex.read(".")) {
		auto index = nameexpression(lex).name;
		name = std::make_shared<dotexpression>(name, index);
	}

	children.push_back(name);

	if (lex.read(":")) {
		method = nameexpression(lex).name;
	}

	body = std::make_shared<funcbody>(lex);
}

string functionstatement::tostring() {
	std::stringstream stream;
	stream << "function " << name->tostring();
	if (method) {
		stream << ":" << *method;
	}

	return stream.str();
}