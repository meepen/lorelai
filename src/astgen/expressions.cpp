#include "expressions.hpp"
#include "visitor.hpp"
#include "errors.hpp"

using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;

#define acceptmacro(name) bool name ::accept(visitor &visit, std::shared_ptr<node> &container) { visit.visit(*this, container); }
LORELAI_EXPRESSION_LITERAL_CLASS_MACRO(acceptmacro);

std::shared_ptr<expression> expression::read(lexer &lex) {
	auto word_or_null = lex.lookahead();
	if (!word_or_null) {
		throw error::expected_for("value", "expression", "no value");
	}

	auto word = word_or_null.value();

	std::shared_ptr<expression> expr;

	if (lexer::isnumberstart(word[0]) && word != ".") {
		expr = std::make_shared<numberexpression>(std::stod(word, nullptr));
		lex.read();
	}

	return expr;
}