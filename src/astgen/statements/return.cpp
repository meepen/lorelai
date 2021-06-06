#include "statements.hpp"
#include "expressions.hpp"
#include "errors.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;

returnstatement::returnstatement(lexer &lex) {
	// consume return
	lex.read();

	// try to read explist
	while (std::shared_ptr<node> exp = expression::read(lex)) {
		children.push_back(exp);
		auto next = lex.lookahead().value_or("");
		if (next != ",") {
			break;
		}
		// consume ','
		lex.read();
	}
}
