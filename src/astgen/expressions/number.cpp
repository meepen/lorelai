#include "astgen/expressions.hpp"

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;

numberexpression::numberexpression(lexer &lex) {
	size_t size;
	auto word = lex.read();
	if (word.size() >= 2 && word[0] == '0') {
		char typ = word[1];
		if (typ == 'x') {
			char *endptr;
			data = std::strtod(word.c_str(), &endptr);
			if ((endptr - word.c_str()) != word.size()) {
				throw error::expected_for("<number>", "number", word.substr(endptr - word.c_str()));
			}
			size = word.size();
		}
		else if (typ == 'b') {
			data = static_cast<number>(std::stol(word.substr(2), &size, 2));
			size += 2;
		}
		else {
			throw error::expected_for("<number>", "number", word);
		}
	}
	else {
		data = std::stod(word, &size);
	}

	if (size != word.size()) {
		throw error::expected_for("<number>", "number", word);
	}
}