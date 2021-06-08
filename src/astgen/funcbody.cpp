#include "funcbody.hpp"
#include "lexer.hpp"
#include "expressions.hpp"
#include "chunks.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::astgen;

funcbody::funcbody(lexer &lex) {
	/*
		parlist ::= namelist [`,´ `...´] | `...´
		funcbody ::= `(´ [parlist] `)´ block end
	*/

	lex.expect("(", "funcbody");

	while (true) {
		auto param = expression::read(lex);
		if (!param) {
			break;
		}

		params.push_back(param);
		children.push_back(param);

		if (dynamic_cast<expressions::varargexpression *>(param.get())) {
			break;
		}
		else if (!dynamic_cast<expressions::nameexpression *>(param.get())) {
			lex.wasexpected("<name or vararg>", "funcbody");
		}
	}

	lex.expect(")", "funcbody");

	block = std::make_shared<chunk>(lex);

	children.push_back(block);

	lex.expect("end", "funcbody");
}

bool funcbody::accept(visitor &visit, std::shared_ptr<node> &container) {
	if (visit.visit(*this, container)) {
		return true;
	}

	visitchildren(visit);
	return false;
}