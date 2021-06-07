#include "expressions.hpp"
#include "lexer.hpp"
#include "errors.hpp"
#include <algorithm>

using namespace lorelai;
using namespace lorelai::astgen;
using namespace lorelai::astgen::expressions;


static string unicodecodepointtoutf8(long codepoint) {
	std::vector<string::value_type> unicodechar;
	if (codepoint < 0) {
		throw error::unexpected_for("invalid codepoint", "unicode codepoint");
	}
	if (codepoint < 0x80) {
		return string(1, codepoint);
	}
	std::vector<unsigned char> utf8;
	unsigned char stoppoint = 0b01000000;
	while (codepoint > stoppoint && stoppoint != 0) {
		utf8.push_back(0b10000000 | (codepoint & 0b00111111) & 0xff);
		stoppoint >>= 1;
		codepoint >>= 6;
	}

	utf8.push_back(~stoppoint & 0xff & ~(stoppoint - 1) | codepoint);
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
			throw error::expected_for("{", "string unicode escape", string(1, next_chr));
		}

		size_t size;
		if (lex.futuredata()[0] == '0' && lex.futuredata()[1] == 'x') {
			throw error::unexpected_for("unicode codepoint", "string unicode escape");
		}
		auto codepoint = std::stol(lex.futuredata(), &size, 16);
		for (size_t i = 0; i < size; i++) {
			lex.readchar();
		}

		next_chr = lex.readchar();
		if (next_chr != '}') {
			throw error::expected_for("}", "string unicode escape", string(1, next_chr));
		}

		return unicodecodepointtoutf8(codepoint);
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
	
	throw error::expected_for("<escape character>", "string", string(1, next_chr));
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
		// literals are /\[=*\]/ 

		// lexer can return '=='
		if (lex.lookahead().value_or("")[0] == '=') {
			long_string_depth += lex.read().size();
		}

		auto got = lex.read();
		if (got != "[") {
			throw error::expected_for("[", "literal string", got);
		}
	}
	else {
		throw error::expected_for("<string>", "string expression", begin);
	}

	// read string data
	std::vector<string::value_type> contents;

	while (true) {
		auto chr = lex.readchar();
		if (chr == '\\' && string_type != LITERAL) {
			auto value = escape(lex, string_type == SINGLE_QUOTE ? '\'': '"');

			contents.insert(contents.end(), value.begin(), value.end());
		}
		else if (chr == '\'' && string_type == SINGLE_QUOTE || chr == '"' && string_type == DOUBLE_QUOTE) {
			break;
		}
		else {
			contents.push_back(chr);
		}
	}

	data = std::string(contents.begin(), contents.end());

	// all other string types are terminated already, we need to consume the last /=*\]/ for literals
	if (string_type == LITERAL) {
		for (size_t i = 0; i < long_string_depth; i++) {
			auto chr = lex.readchar();
			if (chr != '=') {
				throw error::expected_for("=", "literal string", string(1, chr));
			}
		}

		auto end = lex.readchar();
		if (end != ']') {
			throw error::expected_for("]", "literal string", string(1, end));
		}
	}
}