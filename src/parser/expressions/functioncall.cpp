#include "expressions.hpp"
#include "lexer.hpp"
#include "args.hpp"
#include "visitor.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

bool functioncallexpression::applicable(lexer &lex) {
	if (!lex.lookahead()) {
		return false;
	}
	return lex.lookahead().value() == ":" || args::applicable(lex);
}

// functioncall ::=  prefixexp args | prefixexp `:´ Name args 

functioncallexpression::functioncallexpression(node *prefixexp, lexer &lex) {
	funcexpr = prefixexp;
	if (!funcexpr) {
		lex.wasexpected("<prefixexp>", "function call expression");
	}

	if (lex.read(":")) {
		methodname = nameexpression(lex).name;
	}

	arglist = new args(lex);
}

LORELAI_ACCEPT_BRANCH(functioncallexpression)

string functioncallexpression::tostring() {
	std::stringstream stream;

	stream << funcexpr->tostring();
	if (methodname) {
		stream << ":" << *methodname;
	}

	stream << "(" << arglist->tostring() << ")";

	return stream.str();
}