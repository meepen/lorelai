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

	name = new nameexpression(lex);
	while (lex.read(".")) {
		auto index = nameexpression(lex).name;
		name = new dotexpression(name, index);
	}

	if (lex.read(":")) {
		method = nameexpression(lex).name;
	}

	block = new funcbody(lex, method);
}

string functionstatement::tostring() {
	std::stringstream stream;
	stream << "function " << name->tostring();
	if (method) {
		stream << ":" << *method;
	}

	return stream.str();
}