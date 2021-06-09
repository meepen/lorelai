#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include "funcbody.hpp"
#include "chunks.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;
using namespace lorelai::astgen::expressions;

forinstatement::forinstatement(std::shared_ptr<node> _itername, lexer &lex) {
	// for is consumed in for for deducer
	// lex.expect("for", "for .. = .., .., [..] do .. end");

	children.push_back(_itername);
	iternames.push_back(_itername);

	while (lex.read(",")) {
		auto name = std::make_shared<nameexpression>(lex);
		children.push_back(name);
		iternames.push_back(name);
	}

	lex.expect("in", "for ..{, ..} in .. do .. end");

	inexpr = expression::read(lex);

	if (!inexpr) {
		lex.wasexpected("<expression>", "for ..{, ..} in .. do .. end");
	}
	children.push_back(inexpr);

	lex.expect("do", "for .. = .., ..[, ..] do .. end");

	block = std::make_shared<chunk>(lex);
	children.push_back(block);
	
	lex.expect("end", "for .. = .., ..[, ..] do .. end");
}