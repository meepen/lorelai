#include "expressions.hpp"
#include "lexer.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

bool binopexpression::accept(visitor &visit, std::shared_ptr<node> &container, void *data) {
	if (visit.visit(*this, container, data)) {
		return true;
	}

	visitchildren(visit, data);

	return false;
}

const std::vector<std::pair<bool, std::vector<string>>> binopexpression::priorities = {
	{ true, { "^" } },
	{ false, { } }, // unary
	{ false, { "*", "/", "%" } },
	{ false, { "+", "-", } },
	{ true, { ".." } },
	{ false, { "<", ">", "<=", ">=", "==", "~=" } },
	{ false, { "and" } },
	{ false, { "or" } }
};

const std::unordered_map<string, int> binopexpression::prioritymap = {
	{ "^", 0 },

	// unary op

	{ "*", 2 },
	{ "/", 2 },
	{ "%", 2 },

	{ "+", 3 },
	{ "-", 3 },

	{ "..", 4 },
	
	{ "<", 5 },
	{ "<=", 5 },
	{ ">=", 5 },
	{ ">", 5 },
	{ "==", 5 },
	{ "~=", 5 },

	{ "and", 6 },

	{ "or", 7 }
};