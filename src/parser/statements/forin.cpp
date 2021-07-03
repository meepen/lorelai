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

forinstatement::forinstatement(string _itername, lexer &lex) {
	// for is consumed in for for deducer
	// lex.expect("for", "for .. = .., .., [..] do .. end");

	iternames.push_back(_itername);

	while (lex.read(",")) {
		iternames.push_back(nameexpression(lex).name);
	}

	lex.expect("in", "for ..{, ..} in .. do .. end");

	while (true) {
		auto inexpr = expression::read(lex);

		if (!inexpr) {
			if (inexprs.size() == 0) {
				lex.wasexpected("<expression>", "for ..{, ..} in .. do .. end");
			}
			break;
		}
		inexprs.push_back(inexpr);
		children.push_back(inexpr);
	}

	lex.expect("do", "for ..{, ..} in .. do .. end");

	block = std::make_shared<chunk>(lex);
	children.push_back(block);
	
	lex.expect("end", "for ..{, ..} in .. do .. end");
}

string forinstatement::tostring() {
	std::stringstream stream;
	stream << "for ";
	bool first = true;
	for (auto &iter : iternames) {
		if (!first) {
			stream << ", ";
		}
		first = false;
		stream << iter;
	}
	stream << " in ";

	first = true;
	for (auto &inexpr : inexprs) {
		if (!first) {
			stream << ", ";
		}
		first = false;
		stream << inexpr->tostring();
	}

	stream << " do";

	return stream.str();
}