#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;

returnstatement::returnstatement(lexer &lex) {
	lex.expect("return", "return ..");

	// try to read explist
	while (std::shared_ptr<node> exp = expression::read(lex)) {
		children.push_back(exp);
		if (!lex.read(",")) {
			break;
		}
	}
}
