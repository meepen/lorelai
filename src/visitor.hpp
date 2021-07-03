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
#define LORELAI_POSTVISIT_FUNCTION(name) void postvisit(lorelai::parser:: name &obj, std::shared_ptr<lorelai::parser::node> &container) override
#define LORELAI_POSTVISIT_DEFINE(clasname, type) void clasname::postvisit(lorelai::parser:: type &obj, std::shared_ptr<lorelai::parser::node> &container)
#define LORELAI_VISIT_MACRO(name) virtual bool visit(lorelai::parser:: name &obj, std::shared_ptr<lorelai::parser::node> &container) { return false; }
#define LORELAI_POSTVISIT_MACRO(name) virtual void postvisit(lorelai::parser:: name &obj, std::shared_ptr<lorelai::parser::node> &container) { }
#define LORELAI_VISIT_BRANCH_DEFINE(name) \
	bool name::accept(visitor &visit, std::shared_ptr<lorelai::parser::node> &container) { \
		if (!visit.visit(*this, container)) { \
			visitchildren(visit); \
			visit.postvisit(*this, container); \
			return false; \
		} \
 \
		return true; \
	}

#define LORELAI_VISIT_NODE_DEFINE(name) \
	bool name::accept(visitor &visit, std::shared_ptr<lorelai::parser::node> &container) { \
		if (!visit.visit(*this, container)) { \
			visit.postvisit(*this, container); \
			return false; \
		} \
		return true; \
	}


#define LORELAI_VISIT_NAME_MACRO(fn) \
	LORELAI_EXPRESSION_CLASS_MACRO(fn) \
	LORELAI_STATEMENT_CLASS_MACRO(fn) \
	fn(chunk) \
	fn(funcbody) \
	fn(args)

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