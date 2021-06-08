#ifndef VISITOR_HPP_
#define VISITOR_HPP_

#include <memory>
#include "astgen.hpp"
#include "astgen/chunks.hpp"
#include "astgen/statements.hpp"
#include "astgen/expressions.hpp"
#include "astgen/funcbody.hpp"

#define LORELAI_VISIT_MACRO(name) virtual bool visit(name &_node, std::shared_ptr<node> &container) { return false; }
#define LORELAI_VISIT_NAME_MACRO(fn) \
	LORELAI_EXPRESSION_CLASS_MACRO(fn) \
	LORELAI_STATEMENT_CLASS_MACRO(fn) \
	fn(lorelai::astgen::chunk) \
	fn(lorelai::astgen::funcbody)

namespace lorelai {
	namespace astgen {
		// return true to delete a node
		class visitor {
		public:
			LORELAI_VISIT_NAME_MACRO(LORELAI_VISIT_MACRO)
		};
	}
}

#undef LORELAI_VISIT_MACRO

#endif // VISITOR_HPP_