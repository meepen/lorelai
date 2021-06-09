#include "statements.hpp"
#include "lexer.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;

breakstatement::breakstatement(lexer &lex) {
	lex.expect("break", "break");
}

bool breakstatement::accept(visitor &visit, std::shared_ptr<node> &container) {
	return visit.visit(*this, container);
}