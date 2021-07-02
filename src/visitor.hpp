#ifndef VISITOR_HPP_
#define VISITOR_HPP_

#include <memory>
#include "parser.hpp"
#include "parser/chunks.hpp"
#include "parser/statements.hpp"
#include "parser/expressions.hpp"
#include "parser/funcbody.hpp"
#include "parser/args.hpp"



#define LORELAI_VISIT_FUNCTION(name) bool visit(lorelai::parser:: name &obj, std::shared_ptr<lorelai::parser::node> &container) override
#define LORELAI_VISIT_DEFINE(clasname, type) bool clasname::visit(lorelai::parser:: type &obj, std::shared_ptr<lorelai::parser::node> &container)
#define LORELAI_POSTVISIT_FUNCTION(name) bool postvisit(lorelai::parser:: name &obj, std::shared_ptr<lorelai::parser::node> &container) override
#define LORELAI_POSTVISIT_DEFINE(clasname, type) bool clasname::postvisit(lorelai::parser:: type &obj, std::shared_ptr<lorelai::parser::node> &container)
#define LORELAI_VISIT_MACRO(name) virtual bool visit(name &_node, std::shared_ptr<lorelai::parser::node> &container) { return false; }
#define LORELAI_POSTVISIT_MACRO(name) virtual bool postvisit(name &_node, std::shared_ptr<lorelai::parser::node> &container) { return false; }
#define LORELAI_VISIT_BRANCH_DEFINE(name) \
	bool name::accept(visitor &visit, std::shared_ptr<lorelai::parser::node> &container) { \
		bool r; \
		if (!(r = visit.visit(*this, container))) { \
			visitchildren(visit); \
		} \
 \
		bool r2 = visit.postvisit(*this, container); \
 \
		return r || r2; \
	}

#define LORELAI_VISIT_NODE_DEFINE(name) \
	bool name::accept(visitor &visit, std::shared_ptr<lorelai::parser::node> &container) { \
		bool r = visit.visit(*this, container); \
		bool r2 = visit.postvisit(*this, container); \
		return r || r2; \
	}


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
			LORELAI_VISIT_NAME_MACRO(LORELAI_POSTVISIT_MACRO)
		};
	}
}

#undef LORELAI_VISIT_MACRO
#undef LORELAI_POSTVISIT_MACRO

#endif // VISITOR_HPP_