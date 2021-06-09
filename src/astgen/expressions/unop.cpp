#include "expressions.hpp"
#include "lexer.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;

bool unopexpression::accept(visitor &visit, std::shared_ptr<node> &container) {
	if (visit.visit(*this, container)) {
		return true;
	}

	visitchildren(visit);

	return false;
}