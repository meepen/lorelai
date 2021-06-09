#include "expressions.hpp"
#include "lexer.hpp"
#include "args.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;

bool functioncallexpression::applicable(lexer &lex) {
	return args::applicable(lex);
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


bool functioncallexpression::accept(visitor &visit, std::shared_ptr<node> &container) {
	if (visit.visit(*this, container)) {
		return true;
	}

	visitchildren(visit);

	return false;
}
