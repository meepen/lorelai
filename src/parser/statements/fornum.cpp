#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include "funcbody.hpp"
#include "chunks.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;
using namespace lorelai::parser::expressions;

fornumstatement::fornumstatement(string _itername, lexer &lex) : itername(_itername) {
	// for is consumed in for deducer
	// lex.expect("for", "for .. = .., .., [..] do .. end");

	lex.expect("=", "for .. = .., ..[, ..] do .. end");

	startexpr = expression::read(lex);
	lex.expect(",", "for .. = .., ..[, ..] do .. end");

	endexpr = expression::read(lex);
	if (lex.read(",")) {
		stepexpr = expression::read(lex);
	}

	lex.expect("do", "for .. = .., ..[, ..] do .. end");

	block = new chunk(lex);
	
	lex.expect("end", "for .. = .., ..[, ..] do .. end");
}

string fornumstatement::tostring() {
	std::stringstream stream;
	stream << "for " << itername << " = " << startexpr->tostring() << ", " << endexpr->tostring();
	if (stepexpr) {
		stream << ", " << stepexpr->tostring();
	}

	stream << "do";

	return stream.str();
}