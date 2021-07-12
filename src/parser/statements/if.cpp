#include "statements.hpp"
#include "expressions.hpp"
#include "chunks.hpp"
#include "lexer.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;

elseifstatement::elseifstatement(lexer &lex) {
	conditional = expression::read(lex);
	if (!conditional) {
		lex.wasexpected("<expression>", "if .. then .. elseif .. end");
	}

	lex.expect("then", "if .. then .. elseif .. end");
	
	block = new chunk(lex);
}

elsestatement::elsestatement(lexer &lex) {
	block = new chunk(lex);
}

ifstatement::ifstatement(lexer &lex) {
	lex.expect("if", "if .. then .. end");

	conditional = expression::read(lex);
	if (!conditional) {
		lex.wasexpected("<expression>", "if .. then .. end");
	}

	lex.expect("then", "if .. then .. end");

	block = new chunk(lex);

	while (lex.read("elseif")) {
		auto elseif = new elseifstatement(lex);
		elseifs.push_back(elseif);
	}

	if (lex.read("else")) {
		elseblock = new elsestatement(lex);
	}

	lex.expect("end", "if .. then .. end");
}


string ifstatement::tostring() {
	std::stringstream stream;
	stream << "if " << conditional->tostring() << " then";

	return stream.str();
}

string elseifstatement::tostring() {
	std::stringstream stream;
	stream << "elseif " << conditional->tostring() << " then";

	return stream.str();
}

string elsestatement::tostring() {
	return "else";
}