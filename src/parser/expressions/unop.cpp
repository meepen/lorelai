#include "expressions.hpp"
#include "lexer.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

bool unopexpression::accept(visitor &visit, std::shared_ptr<node> &container, void *data) {
	if (visit.visit(*this, container, data)) {
		return true;
	}

	visitchildren(visit, data);
	return false;
}