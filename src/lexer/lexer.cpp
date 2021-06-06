#include "lexer.hpp"

using namespace lorelai;

string_or_null &lexer::lookahead() {
	if (_lookahead || islookaheadeof()) {
		return _lookahead;
	}

	lookahead_posdata = posdata;
	skipwhite();

	if (islookaheadeof()) {
		return _lookahead;
	}
	
	// sometimes the data is just one character, if it's not we update it later
	string next_data = string(1, data[lookahead_posdata.position]);
	string::value_type chr = data[lookahead_posdata.position];

	if (chr == '=') {
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
	_lookahead = next_data;

	return _lookahead;
}