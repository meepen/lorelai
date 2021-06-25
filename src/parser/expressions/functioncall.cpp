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

functioncallexpression::functioncallexpression(std::shared_ptr<node> prefixexp, lexer &lex) {
	funcexpr = prefixexp;
	if (!funcexpr) {
		lex.wasexpected("<prefixexp>", "function call expression");
	}
	children.push_back(funcexpr);

	if (lex.read(":")) {
		methodname = std::make_shared<nameexpression>(lex);
		children.push_back(methodname);
	}

	arglist = std::make_shared<args>(lex);
	children.push_back(arglist);
}

LORELAI_VISIT_BRANCH_DEFINE(functioncallexpression)

string functioncallexpression::tostring() {
	std::stringstream stream;

	stream << funcexpr->tostring();
	if (methodname) {
		stream << ":" << methodname->tostring();
	}

	stream << "(" << arglist->tostring() << ")";

	return stream.str();
}