#include "chunks.hpp"
#include <algorithm>
#include "statements.hpp"
#include "visitor.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;

chunk::chunk(string data, bool expect_eof) {
	lexer lex(data);
	initialize(lex);
	if (expect_eof && !lex.iseof()) {
		lex.wasexpected("<eof>", "chunk");
	}
}

chunk::chunk(lexer &lex) {
	initialize(lex);
}

void chunk::initialize(lexer &lex) {
	while (auto statement = statement::read(lex)) {
		children.push_back(statement);
		if (dynamic_cast<astgen::statement *>(statement.get())->isfinal()) {
			break;
		}
		lex.skipwhite();
	}

	lex.skipwhite();
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