#ifndef VISITOR_HPP_
#define VISITOR_HPP_

#include <memory>
#include "parser.hpp"
#include "parser/chunks.hpp"
#include "parser/statements.hpp"
#include "parser/expressions.hpp"
#include "parser/funcbody.hpp"
#include "parser/args.hpp"

#define LORELAI_VISIT_MACRO(name) virtual bool visit(name &_node, std::shared_ptr<node> &container, void *data) { return false; }
#define LORELAI_VISIT_NAME_MACRO(fn) \
	LORELAI_EXPRESSION_CLASS_MACRO(fn) \
	LORELAI_STATEMENT_CLASS_MACRO(fn) \
	fn(lorelai::parser::chunk) \
	fn(lorelai::parser::funcbody) \
	fn(lorelai::parser::args)

namespace lorelai {
	namespace parser {
		// return true to delete a node
		class visitor {
		public:
			LORELAI_VISIT_NAME_MACRO(LORELAI_VISIT_MACRO)
		};
	}
}

#undef LORELAI_VISIT_MACRO

#endif // VISITOR_HPP_