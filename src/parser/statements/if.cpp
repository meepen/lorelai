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
	children.push_back(conditional);

	lex.expect("then", "if .. then .. elseif .. end");
	
	block = std::make_shared<chunk>(lex);
	children.push_back(block);
}

elsestatement::elsestatement(lexer &lex) {
	block = std::make_shared<chunk>(lex);
	children.push_back(block);
}

ifstatement::ifstatement(lexer &lex) {
	lex.expect("if", "if .. then .. end");

	conditional = expression::read(lex);
	if (!conditional) {
		lex.wasexpected("<expression>", "if .. then .. end");
	}
	children.push_back(conditional);

	lex.expect("then", "if .. then .. end");

	block = std::make_shared<chunk>(lex);
	children.push_back(block);

	while (lex.read("elseif")) {
		auto elseif = std::make_shared<elseifstatement>(lex);
		elseifs.push_back(elseif);
		children.push_back(elseif);
	}

	if (lex.read("else")) {
		elseblock = std::make_shared<elsestatement>(lex);
		children.push_back(elseblock);
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