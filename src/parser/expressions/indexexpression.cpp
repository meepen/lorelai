#include "expressions.hpp"
#include "lexer.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;


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


bool indexexpression::accept(visitor &visit, std::shared_ptr<node> &container, void *data) {
	if (visit.visit(*this, container, data)) {
		return true;
	}

	visitchildren(visit, data);
	return false;
}

bool dotexpression::accept(visitor &visit, std::shared_ptr<node> &container, void *data) {
	if (visit.visit(*this, container, data)) {
		return true;
	}

	visitchildren(visit, data);
	return false;
}