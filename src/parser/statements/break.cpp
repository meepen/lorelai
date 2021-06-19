#include "statements.hpp"
#include "lexer.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;

breakstatement::breakstatement(lexer &lex) {
	lex.expect("break", "break");
}

bool breakstatement::accept(visitor &visit, std::shared_ptr<node> &container, void *data) {
	return visit.visit(*this, container, data);
}