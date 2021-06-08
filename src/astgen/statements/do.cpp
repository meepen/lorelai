#include "statements.hpp"
#include "expressions.hpp"
#include "chunks.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::statements;

dostatement::dostatement(lexer &lex) {
	lex.expect("do", "do .. end");

	block = std::make_shared<chunk>(lex);
	children.push_back(block);

	lex.expect("end", "do .. end");
}
