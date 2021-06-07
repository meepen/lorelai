#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;

dostatement::dostatement(lexer &lex) {
	lex.expect("do", "do .. end");

	while (auto stmt = statement::read(lex)) {
		statements.push_back(stmt);
		children.push_back(stmt);
	}

	lex.expect("end", "do .. end");
}
