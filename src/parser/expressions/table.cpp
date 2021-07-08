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

				hashpart.push_back(std::make_pair(key, value));
				children.push_back(key);
				children.push_back(value);
			}
			else {
				// Name `=` exp | exp
				std::shared_ptr<node> key;
				auto value = expression::read(lex);
				if (!value) {
					break;
				}

				word = lex.lookahead().value_or("");
				
				if (word == "=" && dynamic_cast<nameexpression *>(value.get())) { // TODO: remove nameexpression
					// consume `=`
					lex.read();
 
					children.push_back(value);

					key = std::make_shared<stringexpression>(dynamic_cast<nameexpression *>(value.get())->name);
					value = expression::read(lex);
				}
				else {
					key = std::make_shared<numberexpression>(++arraysize);
				}
				
				children.push_back(value);
				arraypart.push_back(value);
			}
		}
		while (lex.read(",") || lex.read(";"));
	}

	// consume `}`
	lex.expect("}", "table constructor");
}

bool tableexpression::accept(visitor &visit, std::shared_ptr<node> &container) {
	bool r;
	if (!(r = visit.visit(*this, container))) {
		// cannot visit children with this function since it can delete a key and not a value
		// also it does not update tabledata structure
		// visitchildren(visit);

		std::vector<std::shared_ptr<node>> deleted;
		std::vector<std::pair<std::shared_ptr<node>, std::shared_ptr<node>>> keys;

		for (auto &child : children) {
			if (child->accept(visit, child)) {
				deleted.push_back(child);
			}
		}

		for (auto &pair : hashpart) {
			auto firstdeleted = std::find(deleted.begin(), deleted.end(), pair.first);
			auto seconddeleted = std::find(deleted.begin(), deleted.end(), pair.second);

			if (firstdeleted != children.end() && seconddeleted == children.end()) {
				deleted.push_back(pair.second);
			}
			else if (firstdeleted == children.end() && seconddeleted != children.end()) {
				deleted.push_back(pair.first);
			}

			if (firstdeleted != children.end() || seconddeleted != children.end()) {
				keys.push_back(pair);
			}
		}

		for (auto &key : keys) {
			hashpart.erase(std::remove(hashpart.begin(), hashpart.end(), key), hashpart.end());
		}

		for (auto &del : deleted) {
			arraypart.erase(std::remove(arraypart.begin(), arraypart.end(), del));
			children.erase(std::remove(children.begin(), children.end(), del), children.end());
		}

		visit.postvisit(*this, container);
	}

	return r;
}

string tableexpression::tostring() {
	return "{lol}";
}