#include "expressions.hpp"
#include "visitor.hpp"
#include "lexer.hpp"
#include <algorithm>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;


tableexpression::tableexpression(lexer &lex) {
	/*
	tableconstructor ::= `{´ [fieldlist] `}´

	fieldlist ::= field {fieldsep field} [fieldsep]

	field ::= `[´ exp `]´ `=´ exp | Name `=´ exp | exp

	fieldsep ::= `,´ | `;´
	*/
	lex.expect("{", "table expression");

	size_t arraysize = 0;

	auto word = lex.lookahead().value_or("");
	if (word != "" && word != "}") {
		do {
			word = lex.lookahead().value_or("");

			if (word == "[") { // TODO: maybe make this a sub-expression?
				// `[` exp `]` `=` exp
				// consume `[`
				lex.read();
				auto key = expression::read(lex);
				if (!key) {
					lex.wasexpected("<expression>", "table constructor");
				}

				lex.expect("]", "table");
				lex.expect("=", "table");

				auto value = expression::read(lex);
				if (!value) {
					lex.wasexpected("<expression>", "table constructor");
				}

				auto pair = std::make_pair(key, value);

				hashpart.push_back(std::make_pair(key, value));
			}
			else {
				// Name `=` exp | exp
				std::shared_ptr<node> key;
				auto value = expression::read(lex);
				if (!value) {
					break;
				}

				word = lex.lookahead().value_or("");
				
				if (word == "=" && dynamic_cast<nameexpression *>(value)) { // TODO: remove nameexpression
					// consume `=`
					lex.read();

					key = std::make_shared<stringexpression>(dynamic_cast<nameexpression *>(value)->name);
					value = expression::read(lex);
				}
				else {
					key = std::make_shared<numberexpression>(++arraysize);
				}

				arraypart.push_back(value);
			}
		}
		while (lex.read(",") || lex.read(";"));
	}

	// consume `}`
	lex.expect("}", "table constructor");
}

string tableexpression::tostring() {
	return "{}";
}

LORELAI_ACCEPT_BRANCH(tableexpression)