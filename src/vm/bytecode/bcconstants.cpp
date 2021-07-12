#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::parser;

class constantfinder_base : public visitor {
public:
	using visitor::visit;
	using visitor::postvisit;

	LORELAI_VISIT_FUNCTION(statements::assignmentstatement) {
		return false;
	}

	LORELAI_VISIT_FUNCTION(statements::localassignmentstatement) {
		return false;
	}
};