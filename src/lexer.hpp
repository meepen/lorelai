#ifndef LEXER_HPP_
#define LEXER_HPP_

#include <memory>
#include <experimental/optional>
#include "types/types.hpp"


namespace lorelai {
	using string_or_null = std::experimental::optional<string>;
	class lexer {
	private:
		using _posdata = struct {
			size_t position = 0, linenumber = 1, linecolumn = 1;
		};

	public:
		static bool iswhite(string::value_type chr) {
			return chr == ' ' || chr == '\t' || chr == '\r' || chr == '\n' || chr == '\v';
		}

		static bool isalpha(string::value_type chr) {
			return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z');
		}

		static bool isnumeric(string::value_type chr) {
			return chr >= '0' && chr <= '9';
		}

		static bool isnumberstart(string::value_type chr) {
			return isnumeric(chr) || chr == '.';
		}
		
		static bool ispartofname(string::value_type chr) {
			return isalpha(chr) || chr > 0x80 || isnumeric(chr);
		}

	public:
		lexer(string _data) : data(_data) {

		}

		bool iseof() {
			return posdata.position == data.size();
		}

		bool islookaheadeof() {
			return lookahead_posdata.position == data.size();
		}

		size_t skipwhite() {
			size_t begin = lookahead_posdata.position;
			string::value_type chr;
			bool was_r = false;

			while (lookahead_posdata.position < data.size() && iswhite(chr = data[lookahead_posdata.position])) {
				if (chr == '\r' || (chr == '\n' && !was_r)) {
					lookahead_posdata.linenumber++;
					lookahead_posdata.linecolumn = 0;
				}
				lookahead_posdata.linecolumn++;
				was_r = chr == '\r';
				lookahead_posdata.position++;
			}

			return lookahead_posdata.position - begin;
		}

		string_or_null &lookahead() {
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
			else if (isalpha(chr)) {
				auto curpos = lookahead_posdata.position;
				while (data.size() > curpos && ispartofname(data[curpos])) {
					curpos++;
				}
				next_data = data.substr(lookahead_posdata.position, curpos - lookahead_posdata.position);
			}
			else if (isnumberstart(chr)) {
				try {
					size_t size;
					std::stod(&data[lookahead_posdata.position], &size);
					next_data = data.substr(lookahead_posdata.position, size);
				}
				catch (std::exception &e) {
					if (chr == '.') {
						next_data = chr;
					}
					else {
						throw e;
					}
				}
			}

			lookahead_posdata.position += next_data.size();
			_lookahead = next_data;

			return _lookahead;
		}

		const string::value_type *futuredata() {
			return data.data() + posdata.position;
		}

		string read() {
			auto ret = lookahead();
			posdata = lookahead_posdata;
			_lookahead = {};
			// do we need to do this here?
			// read_newline_r = false;
			
			return ret.value_or("");
		}

		string::value_type peekchar() {
			return data[posdata.position];
		}

		string::value_type readchar() {
			// since we aren't using lookahead anymore, invalidate
			_lookahead = {};

			auto chr = data[posdata.position];
			if (chr == '\r') {
				read_newline_r = true;
			}
			else {
				if (chr == '\n' && !read_newline_r) {
					posdata.linenumber++;
					posdata.linecolumn = 0;
				}
				read_newline_r = false;
			}

			posdata.linecolumn++;
			posdata.position++;

			return chr;
		}

	private:
		string_or_null _lookahead;
		_posdata lookahead_posdata;

		bool read_newline_r = false;

	public:
		string data;
		_posdata posdata;
	};
}

#endif // LEXER_HPP_