#include "statements.hpp"
#include "expressions.hpp"
#include "lexer.hpp"
#include "funcbody.hpp"
#include "chunks.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;
using namespace lorelai::astgen::expressions;

fornumstatement::fornumstatement(std::shared_ptr<node> _itername, lexer &lex) {
	// for is consumed in for for deducer
	// lex.expect("for", "for .. = .., .., [..] do .. end");

	itername = _itername; // std::make_shared<nameexpression>(lex);
	children.push_back(itername);

	lex.expect("=", "for .. = .., ..[, ..] do .. end");

	startexpr = expression::read(lex);
	children.push_back(startexpr);
	lex.expect(",", "for .. = .., ..[, ..] do .. end");
	endexpr = expression::read(lex);
	children.push_back(endexpr);
	if (lex.read(",")) {
		stepexpr = expression::read(lex);
		children.push_back(stepexpr);
	}

	lex.expect("do", "for .. = .., ..[, ..] do .. end");

	body = std::make_shared<chunk>(lex);
	children.push_back(body);
	
	lex.expect("end", "for .. = .., ..[, ..] do .. end");
}