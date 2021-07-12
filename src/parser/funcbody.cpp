#include "funcbody.hpp"
#include "lexer.hpp"
#include "expressions.hpp"
#include "chunks.hpp"
#include "visitor.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;

funcbody::funcbody(lexer &lex) {
	/*
		parlist ::= namelist [`,´ `...´] | `...´
		funcbody ::= `(´ [parlist] `)´ block end
	*/

	lex.expect("(", "funcbody");

	bool first = true;
	while (true) {
		auto param = expression::read(lex);
		if (!param) {
			if (first) {
				break;
			}
			else {
				lex.wasexpected("<name or vararg>", "funcbody");
			}
		}

		params.push_back(param);

		if (dynamic_cast<expressions::varargexpression *>(param)) {
			break;
		}
		else if (!dynamic_cast<expressions::nameexpression *>(param)) {
			lex.wasexpected("<name or vararg>", "funcbody");
		}

		if (!lex.read(",")) {
			break;
		}
	}

	lex.expect(")", "funcbody");

	block = new chunk(lex);

	lex.expect("end", "funcbody");
}

string funcbody::tostring() {
	std::stringstream stream;
	stream << "(";
	bool first = true;
	for (auto &param : params) {
		if (!first) {
			stream << ", ";
		}
		first = false;
		stream << param->tostring();
	}

	return stream.str();
}

LORELAI_ACCEPT_BRANCH(funcbody)
