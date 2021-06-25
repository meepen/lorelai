#include "expressions.hpp"
#include "lexer.hpp"
#include "visitor.hpp"
#include <sstream>

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

LORELAI_VISIT_BRANCH_DEFINE(indexexpression)
LORELAI_VISIT_BRANCH_DEFINE(dotexpression)

string dotexpression::tostring() {
	std::stringstream stream;
	stream << prefix->tostring() << "." << index->tostring();

	return stream.str();
}

string indexexpression::tostring() {
	std::stringstream stream;
	stream << prefix->tostring() << "[" << index->tostring() << "]";

	return stream.str();
}