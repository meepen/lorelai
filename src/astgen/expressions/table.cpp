#include "expressions.hpp"
#include "errors.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;


tableexpression::tableexpression(lexer &lex) {
	/*
	tableconstructor ::= `{´ [fieldlist] `}´

	fieldlist ::= field {fieldsep field} [fieldsep]

	field ::= `[´ exp `]´ `=´ exp | Name `=´ exp | exp

	fieldsep ::= `,´ | `;´
	*/

	size_t arraysize = 0;

	auto word = lex.lookahead().value_or("");
	if (word != "" && word != "}") {
		do {
			// consume '{' or ','
			lex.read();

			word = lex.lookahead().value_or("");

			if (word == "[") {
				// `[` exp `]` `=` exp
				// consume `[`
				lex.read();
				auto key = expression::read(lex);

				word = lex.read();
				if (word != "]") {
					throw error::expected_for("]", "[<key>]", word);
				}

				word = lex.read();
				if (word != "=") {
					throw error::expected_for("=", "[<key>] = ", word);
				}

				auto value = expression::read(lex);

				tabledata.push_back(std::make_pair(key, value));
				children.push_back(key);
				children.push_back(value);
			}
			else {
				// Name `=` exp | exp
				std::shared_ptr<node> key;
				auto value = expression::read(lex);

				word = lex.lookahead().value_or("");
				
				if (word == "=" && dynamic_cast<nameexpression *>(value.get())) {
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
				tabledata.push_back(std::make_pair(key, value));
			}

			word = lex.lookahead().value_or("");
		}
		while (word == ",");
	}

	// consume `}`
	word = lex.read();

	if (word != "}") {
		throw error::expected_for("}", "table constructor", word);
	}
}

bool tableexpression::accept(visitor &visit, std::shared_ptr<node> &container) {
	if (visit.visit(*this, container)) {
		return true;
	}

	// cannot visit children with this function since it can delete a key and not a value
	// also it does not update tabledata structure
	// visitchildren(visit);

	std::vector<std::shared_ptr<node>> deleted;
	std::vector<std::pair<std::shared_ptr<node>, std::shared_ptr<node>>> keys;

	for (auto &child : children) {
		deleted.push_back(child);
	}

	for (auto &pair : tabledata) {
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
		tabledata.erase(std::remove(tabledata.begin(), tabledata.end(), key), tabledata.end());
	}

	for (auto &del : deleted) {
		children.erase(std::remove(children.begin(), children.end(), del), children.end());
	}

	return false;
}