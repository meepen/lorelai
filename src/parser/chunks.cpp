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
		childstatements.push_back(statement);
		if (dynamic_cast<parser::statement *>(statement)->isfinal()) {
			break;
		}
	}
}

LORELAI_ACCEPT_BRANCH(chunk)
