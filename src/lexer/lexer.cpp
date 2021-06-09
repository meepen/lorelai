#include "lexer.hpp"
#include <algorithm>
#include <unordered_set>
#include <exception>

using namespace lorelai;

static optional<string> empty = {};

optional<string> &lexer::lookahead(bool shouldskip) {
	if (_lookahead || islookaheadeof()) {
		return _lookahead;
	}

	if (shouldskip) {
		lookahead_posdata = posdata;
		skipwhite();
	}

	if (islookaheadeof()) {
		posdata = lookahead_posdata;
		return _lookahead;
	}

	// do NOT save whitespace in lookahead
	if (!shouldskip && iswhite(data[lookahead_posdata.position])) {
		return empty;
	}
	
	// sometimes the data is just one character, if it's not we update it later
	string next_data = string(1, data[lookahead_posdata.position]);
	string::value_type chr = data[lookahead_posdata.position];

	if ((chr == '>' || chr == '<') && !islookaheadeof() && data[lookahead_posdata.position + 1] == '=') {
		next_data = data.substr(lookahead_posdata.position, 2);
	}
	else if (chr == '=') {
		size_t endpos;
		for (endpos = lookahead_posdata.position; endpos < data.size(); endpos++) {
			if (data[endpos] != '=') {
				break;
			}
		}

		next_data = data.substr(lookahead_posdata.position, endpos - lookahead_posdata.position);
	}
	else if (chr == '.') {
		auto startpos = lookahead_posdata.position;
		auto curpos = startpos + 1;
		for (size_t i = 0; i < 2; i++) {
			if (data[curpos] != '.') {
				break;
			}
			curpos++;
		}

		next_data = data.substr(startpos, curpos - startpos);
	}
	else if (isalpha(chr) || isnumeric(chr)) {
		auto curpos = lookahead_posdata.position;
		bool has_decimal = !isnumeric(chr);
		while (data.size() > curpos && (ispartofname(data[curpos]) || !has_decimal && data[curpos] == '.')) {
			has_decimal = chr == '.' ? true : has_decimal;
			curpos++;
		}
		next_data = data.substr(lookahead_posdata.position, curpos - lookahead_posdata.position);
	}

	lookahead_posdata.position += next_data.size();
	lookahead_posdata.linecolumn += next_data.size();
	_lookahead = next_data;

	return _lookahead;
}


const std::unordered_set<string> keywords = {
	"and",
	"break",
	"do",
	"else",
	"elseif",
	"end",
	"false",
	"for",
	"function",
	"goto",
	"if",
	"in",
	"local",
	"nil",
	"not",
	"or",
	"repeat",
	"return",
	"then",
	"true",
	"until",
	"while"
};

bool lexer::iskeyword(string word) {
	return keywords.find(word) != keywords.end();
}


class unexpected_token : public std::exception {
public:
	unexpected_token(lexer &lex, std::string from) {
		error = string(":") + std::to_string(lex.posdata.linenumber) + ":" + std::to_string(lex.posdata.linecolumn) +": unexpected '" + lex.lookahead().value_or("<no value>") + "' while parsing from " + from;
	}
	unexpected_token(lexer &lex, std::string what, std::string from) {
		error = string(":") + std::to_string(lex.posdata.linenumber) + ":" + std::to_string(lex.posdata.linecolumn) + ": expected '" + what + + "' while parsing from " + from;
	}
	const char *what() const noexcept override {
		return error.c_str();
	}

public:
	std::string error;
};

bool lexer::read(string readcondition) {
	if (!lookahead() || lookahead().value() != readcondition) {
		return false;
	}

	read();
	return true;
}

void lexer::expect(string what, string from) {
	if (!lookahead() || lookahead().value() != what) {
		throw unexpected_token(*this, from);
	}

	read();
}

void lexer::wasexpected(string what, string from) {
	throw unexpected_token(*this, what, from);
}
