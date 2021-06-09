#include "expressions.hpp"
#include "lexer.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

numberexpression::numberexpression(lexer &lex) {
	size_t size = 0;
	auto word = lex.read();

	if (word.size() >= 2 && word[0] == '0') {
		char typ = word[1];
		if (typ == 'x') {
			char *endptr;
			data = std::strtod(word.c_str(), &endptr);
			if ((endptr - word.c_str()) != word.size()) {
				lex.wasexpected("<number>", "number");
			}
			size = word.size();
		}
		else if (typ == 'b') {
			data = static_cast<number>(std::stol(word.substr(2), &size, 2));
			size += 2;
		}
	}

	if (size == 0) {
		data = std::stod(word, &size);
	}

	if (size != word.size()) {
		lex.wasexpected("<number>", "number");
	}
}