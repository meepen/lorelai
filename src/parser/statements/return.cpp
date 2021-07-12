#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;

returnstatement::returnstatement(lexer &lex) {
	lex.expect("return", "return ..");

	// try to read explist
	while (auto exp = expression::read(lex)) {
		retlist.push_back(exp);
		if (!lex.read(",")) {
			break;
		}
	}
}

string returnstatement::tostring() {
	std::stringstream stream;
	stream << "return";
	if (retlist.size() > 0) {
		stream << " ";
		bool first = true;

		for (auto &child : retlist) {
			if (!first) {
				stream << ", ";
			}
			first = false;
			stream << child->tostring();
		}
	}

	return stream.str();
}