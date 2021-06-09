#include "statements.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;
using namespace lorelai::parser::expressions;

bool functioncallstatement::accept(visitor &visit, std::shared_ptr<node> &container) {
	if (visit.visit(*this, container)) {
		return true;
	}

	visitchildren(visit);
	return false;
}