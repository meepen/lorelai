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
	while (std::shared_ptr<node> exp = expression::read(lex)) {
		children.push_back(exp);
		if (!lex.read(",")) {
			break;
		}
	}
}

string returnstatement::tostring() {
	std::stringstream stream;
	stream << "return";
	if (children.size() > 0) {
		stream << " ";
		bool first = true;

		for (auto &child : children) {
			if (!first) {
				stream << ", ";
			}
			first = false;
			stream << child->tostring();
		}
	}

	return stream.str();
}