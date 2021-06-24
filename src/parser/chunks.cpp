#include "chunks.hpp"
#include <algorithm>
#include "statements.hpp"
#include "visitor.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::parser;

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
		if (dynamic_cast<parser::statement *>(statement.get())->isfinal()) {
			break;
		}
		lex.skipwhite();
	}

	lex.skipwhite();
}

// visitor acceptors
bool chunk::accept(visitor &visit, std::shared_ptr<node> &container) {
	bool r;
	if (!(r = visit.visit(*this, container))) {
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
	}

	bool r2 = visit.postvisit(*this, container);

	return r || r2;
}