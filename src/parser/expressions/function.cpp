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


bool functionexpression::accept(visitor &visit, std::shared_ptr<node> &container, void *data) {
	if (visit.visit(*this, container, data)) {
		return true;
	}

	visitchildren(visit, data);
	return false;
}
