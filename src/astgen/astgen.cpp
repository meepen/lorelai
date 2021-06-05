#include <algorithm>
#include "astgen.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::astgen;



chunk::chunk(lexer &lex) {
	while (auto statement = statement::read(lex)) {
		children.push_back(statement);
		lex.skipwhite();
	}

	lex.skipwhite();

	if (!lex.iseof()) {
		throw error::expected_for("EOF", "Chunk", lex.lookahead().value());
	}
}

// visitor acceptors

bool chunk::accept(visitor &visit, std::shared_ptr<node> &container) {
	bool ret = visit.visit(*this, container);
	if (ret) { // if we delete who cares, return early
		return true;
	}
	
	std::vector<std::shared_ptr<node>> deleted;

	for (auto &child : children) {
		if (child->accept(visit, child)) {
			// mark for removal
			deleted.push_back(child);
		}
	}
	
	for (auto &del : deleted) {
		children.erase(std::find(children.cbegin(), children.cend(), del));
	}

	return false;
}