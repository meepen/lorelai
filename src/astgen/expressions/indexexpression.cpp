#include "expressions.hpp"
#include "lexer.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;


indexexpression::indexexpression(std::shared_ptr<node> _prefix, lexer &lex) {
	prefix = _prefix;
	children.push_back(prefix);
	lex.expect("[", "index expression");

	index = expression::read(lex);
	if (!index) {
		lex.wasexpected("<expression>", "index expression");
	}
	children.push_back(prefix);

	lex.expect("]", "index expression");
}

dotexpression::dotexpression(std::shared_ptr<node> _prefix, lexer &lex) {
	prefix = _prefix;
	children.push_back(prefix);
	lex.expect(".", "dot expression");

	index = std::make_shared<nameexpression>(lex);
	children.push_back(index);
}


bool indexexpression::accept(visitor &visit, std::shared_ptr<node> &container) {
	if (visit.visit(*this, container)) {
		return true;
	}

	visitchildren(visit);

	return false;
}

bool dotexpression::accept(visitor &visit, std::shared_ptr<node> &container) {
	if (visit.visit(*this, container)) {
		return true;
	}

	visitchildren(visit);

	return false;
}