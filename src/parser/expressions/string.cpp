#include "expressions.hpp"
#include "lexer.hpp"
#include <algorithm>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

static string unicodecodepointtoutf8(lexer &lex, long codepoint) {
	std::vector<string::value_type> unicodechar;
	if (codepoint < 0) {
		lex.wasexpected("valid unicode codepoint", "unicode codepoint");
	}
	if (codepoint < 0x80) {
		return string(1, codepoint);
	}
	std::vector<unsigned char> utf8;
	unsigned char stoppoint = 0b01000000;
	while (codepoint > stoppoint && stoppoint != 0) {
		utf8.push_back(0b10000000 | ((codepoint & 0b00111111) & 0xff));
		stoppoint >>= 1;
		codepoint >>= 6;
	}

	utf8.push_back((~stoppoint & 0xff) & ~(stoppoint - 1) | codepoint);
	std::reverse(utf8.begin(), utf8.end());

	return string(utf8.begin(), utf8.end());
}

static string escape(lexer &lex, string::value_type escape_char) {
	auto next_chr = lex.readchar();
	if (escape_chars.count(next_chr) == 1) {
		return escape_chars.at(next_chr);
	}
	else if (next_chr == escape_char) {
		return string(1, next_chr);
	}
	else if (next_chr == 'u') {
		next_chr = lex.readchar();
		if (next_chr != '{') {
			lex.wasexpected("{", "unicode character");
		}

		size_t size;
		if (lex.futuredata()[0] == '0' && lex.futuredata()[1] == 'x') {
			lex.wasexpected("valid unicode codepoint", "unicode character");
		}
		auto codepoint = std::stol(lex.futuredata(), &size, 16);
		for (size_t i = 0; i < size; i++) {
			lex.readchar();
		}

		next_chr = lex.readchar();
		if (next_chr != '}') {
			lex.wasexpected("}", "unicode character");
		}

		return unicodecodepointtoutf8(lex, codepoint);
	}
	else if (next_chr == 'x') {
		char hex[3];
		hex[0] = lex.readchar();
		hex[1] = lex.readchar();
		hex[2] = 0;
		return string(1, std::stol(hex, nullptr, 16));
	}
	else if (lexer::isnumeric(next_chr)) {
		long chr = next_chr - '0';

		for (size_t i = 1; i < 3; i++) {
			if (!lexer::isnumeric(lex.peekchar())) {
				break;
			}
			long decoded = chr * 10 + lex.peekchar() - '0';
			if (decoded > 255) {
				break;
			}
			chr = decoded;
			lex.readchar();
		}

		return string(1, chr);
	}

	lex.wasexpected("<escape character>", "string");
	throw;
}

stringexpression::stringexpression(lexer &lex) {
	auto begin = lex.read();
	if (begin == "'") {
		string_type = SINGLE_QUOTE;
	}
	else if (begin == "\"") {
		string_type = DOUBLE_QUOTE;
	}
	else if (begin == "[") {
		string_type = LITERAL;

		// ensure no spaces
		if (!lex.lookahead(false)) {
			lex.wasexpected("<literal string>", "string");
		}
		// literals are /\[=*\]/ 

		// lexer can return '====='
		if (lex.lookahead(false).value_or("")[0] == '=') {
			long_string_depth += lex.read().size();
		}
		
		// ensure no spaces
		if (!lex.lookahead(false)) {
			lex.wasexpected("<literal string>", "string");
		}

		lex.expect("[", "string");
	}
	else {
		lex.wasexpected("<string>", "string");
	}

	// read string data
	std::vector<string::value_type> contents;

	while (true) {
		auto chr = lex.readchar();
		if (string_type != LITERAL && chr == '\\') {
			auto value = escape(lex, string_type == SINGLE_QUOTE ? '\'': '"');

			contents.insert(contents.end(), value.begin(), value.end());
		}
		else {
			if (string_type == SINGLE_QUOTE && chr == '\'' || string_type == DOUBLE_QUOTE && chr == '"') {
				break;
			}
			else if (chr == ']' && string_type == LITERAL) {
				bool good = true;
				for (size_t i = 0; i < long_string_depth; i++) {
					auto next = lex.peekchar();
					if (next != '=') {
						string equals('=', i);
						contents.push_back(chr);
						contents.insert(contents.end(), equals.begin(), equals.end());
						good = false;
						break;
					}
					lex.readchar();
				}
				if (!good) {
					continue;
				}

				auto next = lex.peekchar();
				if (next != ']') {
					string equals('=', long_string_depth);
					contents.push_back(chr);
					contents.insert(contents.end(), equals.begin(), equals.end());
					continue;
				}
				lex.readchar();

				break;
			}
			contents.push_back(chr);
		}
	}

	data = std::string(contents.begin(), contents.end());
}

bool stringexpression::applicable(lexer &lex) {
	if (!lex.lookahead()) {
		return false;
	}

	auto word = lex.lookahead().value();
	if (word == "'" || word == "\"") {
		return true;
	}

	if (word == "[") {
		lexer farther = lex;
		farther.read();
		auto next = farther.readchar();
		return next == '[' || next == '=';
	}
	return false;
}

string stringexpression::tostring() {
	return data;
}