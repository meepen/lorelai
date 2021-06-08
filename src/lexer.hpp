#ifndef LEXER_HPP_
#define LEXER_HPP_

#include <memory>
#include "types.hpp"

namespace lorelai {
	class lexer {
	private:
		using _posdata = struct {
			size_t position = 0, linenumber = 1, linecolumn = 1;
		};

	public:
		static bool iswhite(string::value_type chr) {
			return chr == ' ' || chr == '\t' || chr == '\r' || chr == '\n' || chr == '\v' || chr == '\b';
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

		static bool ishexachar(string::value_type chr) {
			return isnumeric(chr) || (chr >= 'a' && chr <= 'f') || (chr >= 'A' && chr <= 'F');
		}

		static bool iskeyword(string word);

		/*
		Names (also called identifiers) in Lua can be any string of letters, digits, and underscores,
		not beginning with a digit. This coincides with the definition of names in most languages.
		(The definition of letter depends on the current locale: any character considered alphabetic
		by the current locale can be used in an identifier.) Identifiers are used to name variables
		and table fields. 
		*/

		static bool isnamestart(string::value_type chr) {
			return isalpha(chr) || (unsigned)(chr) >= 0x80;
		}
		static bool ispartofname(string::value_type chr) {
			return isalpha(chr) || chr > 0x80 || isnumeric(chr) || chr == '_';
		}

		static bool isname(string word) {
			if (word.size() == 0 || iskeyword(word) || !isnamestart(word[0])) {
				return false;
			}
			bool good = true;
			for (size_t i = 1; i < word.size(); i++) {
				if (!lexer::ispartofname(word[i])) {
					good = false;
					break;
				}
			}

			return good;
		}

	public:
		lexer(string _data) : data(_data) { }

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

		optional<string> &lookahead();

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

		void expect(string what, string from);
		void wasexpected(string what, string from);

		bool read(string readcondition);

	private:
		optional<string> _lookahead;
		_posdata lookahead_posdata;

		bool read_newline_r = false;

	public:
		string data;
		_posdata posdata;
	};
}

#endif // LEXER_HPP_