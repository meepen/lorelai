#include "expressions.hpp"
#include "lexer.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

bool unopexpression::accept(visitor &visit, std::shared_ptr<node> &container) {
	if (visit.visit(*this, container)) {
		return true;
	}

	visitchildren(visit);
	return false;
}